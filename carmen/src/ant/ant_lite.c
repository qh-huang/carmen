/*********************************************************
 *
 * This source code is part of the Carnegie Mellon Robot
 * Navigation Toolkit (CARMEN)
 *
 * CARMEN Copyright (c) 2002 Michael Montemerlo, Nicholas
 * Roy, and Sebastian Thrun
 *
 * CARMEN is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public 
 * License as published by the Free Software Foundation; 
 * either version 2 of the License, or (at your option)
 * any later version.
 *
 * CARMEN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General 
 * Public License along with CARMEN; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <netinet/in.h>

#ifndef NO_TCPD
#include <tcpd.h>
#endif

#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "sockutils.h"
#include "carmen/ant.h"

#define          MAX_CALLBACKS          100
#define          MAX_CONNECTIONS        10

#ifdef USE_IPC
#ifdef EXTERNAL_IPC
#include <ipc.h>
#else
#include <carmen/ipc.h>
#endif
#endif

#define ant_test_alloc(X) do {if ((void *)(X) == NULL) {fprintf(stderr, "Out of memory in %s, (%s, line %d).\n", __FUNCTION__, __FILE__, __LINE__); exit(-1);}} while (0)

typedef enum {ANT_SERVER, ANT_CLIENT} ant_direction_t;

int hosts_ctl(char *, char *, char *, char *);
int isblank(int c);
#ifdef USE_READLINE
static void ant_callback_handle_rl_data(char *line);
#endif

#ifndef NO_TCPD
/* allow_severity, deny_severity used by hosts_ctl */
int allow_severity = 5;
int deny_severity = 5;
#endif

typedef struct sockaddr SA;

typedef struct {
  char command[100];
  int locked;
  void (*handler)(int, ant_argument *, int);
} callback;

typedef struct {
  int sock;
  int which;
  ant_direction_t dir;
  char *identity;
  char *prompt;
  char *buffer;
  int buffer_size;
  int num_chars_in_buffer;
} ant_connection;

static char default_prompt [100] = "DEFAULT: ";

static callback callback_list[MAX_CALLBACKS];
static int callback_num = 0;
static void (*default_callback)(int, ant_argument *, int) = NULL;
static void (*null_callback)(int) = NULL;

static ant_connection conn[MAX_CONNECTIONS];
static int connection_num = 0;

static int ant_callback_handle_data(int fd);

double ant_get_time_ms(void)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return tv.tv_sec+tv.tv_usec/1000000.0;
}

static ant_connection *
lookup_connection(int fd)
{
  int i;

  for(i = 0; i < connection_num; i++)
    if(conn[i].sock == fd)
      return conn+i;

  return NULL;
}

int ant_writen(int fd, const void *vptr, int n, int timeout)
{
  return sock_writen(fd, vptr, n, timeout);
}

int ant_readn(int fd, void *vptr, int n, int timeout)
{
  return sock_readn(fd, vptr, n, timeout);
}

int ant_bytes_to_read(int sock)
{
  return sock_bytes_to_read(sock);
}

int ant_write_string(int sock, char *s)
{
  int i, n;

  if(sock == -1) {
    for(i = 0; i < connection_num; i++)
      sock_write_string(conn[i].sock, s);
    return 0;
  }
  else {
    n = sock_write_string(sock, s);
    return n;
  }
}

int ant_printf(int sock, char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
int ant_printf(int sock, char *fmt, ...)
{
  va_list args;
  char Buffer[256];
  int n;

  va_start(args, fmt);
  n = vsnprintf(Buffer, 256, fmt, args);
  va_end(args);
  if (n > -1 && n < 256) {    
    return ant_write_string(sock, Buffer);
  }
  return -1;
}

static int
echo_prompt(ant_connection *c)
{
  int n;

#ifdef USE_READLINE
  if (c->sock != STDIN_FILENO) {
#endif
  n = ant_writen(c->sock, c->prompt, strlen(c->prompt), 1);
  if(n != (signed int) strlen(c->prompt))
    {
      ant_close_connection(c->sock);
      return -1;
    }
#ifdef USE_READLINE
  }
#endif
  
  return 0;
}

static void 
prompt_handler(int ant_argc, ant_argument *ant_argv, int sock) 
{
  int length;
  ant_connection *c;

  if (ant_argc < 2)
    return;

  c = lookup_connection(sock);
  if (!c)
    return;

  length = strlen(ant_argv[1]);
  if (length > 250)
    return;

  if (c->prompt)
    free(c->prompt);
  c->prompt = (char *)calloc(length+1, sizeof(char));  /* check_alloc checked */
  ant_test_alloc(c->prompt);

  strncpy (c->prompt, ant_argv[1], length);
  c->prompt[length] = '\0';

  if (c->prompt[length-2] == '\\' &&
      tolower(c->prompt[length-1]) == 'n') {
    c->prompt[length-2] = '\n';
    c->prompt[length-1] = '\0';
  }
}

static void
quit_handler(int argc __attribute__ ((unused)), 
	     ant_argument *argv __attribute__ ((unused)), int sock)
{ 
  if (sock == STDIN_FILENO)
    kill(getpid(), SIGINT);
  
  ant_close_connection(sock);
}

static void
help_handler(int argc __attribute__ ((unused)), 
	     ant_argument *argv __attribute__ ((unused)), int sock)
{ 
  int callback_index;

  ant_printf(sock, "Accepted commands: \n");  
  for (callback_index = 0; callback_index < callback_num; callback_index++)
    ant_printf(sock, "%s\n", callback_list[callback_index].command);
}

static void
identity_handler(int argc, ant_argument *argv, int sock)
{ 
  if (argc > 1)
    ant_set_identity(sock, argv[1]);

  ant_printf(sock, "identity: %s\n", ant_get_identity(sock));
}

#ifdef USE_READLINE
static char *
command_matches(const char *text, int state)
{
  static int list_index = 0, len = 0;
  char *command;

  if (!state) 
    { 
      list_index = 0;
      len = strlen(text);
    }
    
  for (; list_index < callback_num; list_index++)
    {
      if (strncasecmp(callback_list[list_index].command, text, len) == 0) {
	command = (char *)calloc(strlen(callback_list[list_index].command)+1, sizeof(char)); /* check_alloc checked */
	ant_test_alloc(command);
	strcpy(command, callback_list[list_index++].command);
	return command;
      }
    }

  return NULL;
}

#if READLINE_PRE42
static char **
command_completion(char *text, int start, 
		   int end __attribute__ ((unused)))
#else
static char **
command_completion(const char *text, int start, 
		   int end __attribute__ ((unused)))
#endif
{
  char **matches = NULL;

  if (start == 0)
#if READLINE_PRE42
    matches = completion_matches(text, command_matches);
#else    
    matches = rl_completion_matches(text, command_matches);
#endif

  return matches;
}
#endif

int 
ant_open_server(int port, char *serverName)
{
  int listenfd, err;
  struct sockaddr_in servaddr;
  int arg = 1;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenfd < 0)
    return -1;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)&arg, sizeof(arg));
  err = bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
  if(err < 0)
    return -1;
  err = listen(listenfd, 5);
  if (err < 0)
    return -1;

#ifdef USE_IPC
  IPC_subscribeFD(listenfd, (FD_HANDLER_TYPE)ant_handle_connect, serverName);
#else
  serverName = serverName;
#endif

  ant_register_callback("help", help_handler); 
  ant_register_callback("prompt", prompt_handler);
  ant_register_callback("identity", identity_handler);
  ant_register_callback("quit", quit_handler);
  ant_register_callback("exit", quit_handler);
  
  return listenfd;
}

void 
ant_shutdown(int listenfd)
{
  int conn_index;

  for (conn_index = 0; conn_index < connection_num; conn_index++) 
    ant_close_connection(conn[conn_index].sock);

  if (listenfd >= 0)
    {
      close(listenfd);
#ifdef USE_IPC
      IPC_unsubscribeFD(listenfd, (FD_HANDLER_TYPE)ant_handle_connect);
#endif
    }
}

int
ant_add_connection(int connfd)
{  
  if (connection_num == MAX_CONNECTIONS)
    {
      fprintf(stderr, "Max number of connections is %d : new connection "
	      "not accepted.\n", MAX_CONNECTIONS);
      ant_printf(connfd, "Max number of connections is %d : new connection "
		 "not accepted.\n", MAX_CONNECTIONS);
      close(connfd);
      return -1;
    }
  
  conn[connection_num].sock = connfd;
  conn[connection_num].which = connection_num;
  conn[connection_num].dir = ANT_SERVER;
  
  conn[connection_num].identity = (char *)calloc(8, sizeof(char)); /* check_alloc checked */
  ant_test_alloc(conn[connection_num].identity);
    
  strcpy(conn[connection_num].identity, "DEFAULT");

  conn[connection_num].prompt = 
    (char *)calloc(strlen(default_prompt)+1, sizeof(char)); /* check_alloc checked */
  ant_test_alloc(conn[connection_num].prompt);
  strncpy(conn[connection_num].prompt, default_prompt, strlen(default_prompt));
  conn[connection_num].prompt[strlen(default_prompt)] = '\0';

  conn[connection_num].buffer = NULL;
  conn[connection_num].buffer_size = 0;
  conn[connection_num].num_chars_in_buffer = 0;

#ifdef USE_READLINE	
  if (connfd == STDIN_FILENO) {
    rl_attempted_completion_function = command_completion;
    rl_callback_handler_install(conn[connection_num].prompt, 
				(VFunction *)ant_callback_handle_rl_data);
    using_history();
    stifle_history(100);
  }
#endif

  echo_prompt(conn+connection_num);

#ifdef USE_IPC
#ifdef USE_READLINE
  if (connfd == STDIN_FILENO) {
    IPC_subscribeFD(connfd, (FD_HANDLER_TYPE)rl_callback_read_char, NULL);
  } else
#endif
    IPC_subscribeFD(connfd, (FD_HANDLER_TYPE)ant_callback_handle_data, NULL);
#endif
  
  connection_num++;

  return connfd;
}

int
ant_handle_connect(int listenfd, char *server_name)
{
  int connfd;
  int err;
#ifndef NO_TCPD
  struct sockaddr_in peer;
  int addrlen=sizeof(peer);
  struct hostent *client;
  char *client_machine, *client_hostnum;
#endif
  
  server_name = server_name;

  err = listen(listenfd, 5);
  if (err < 0)
    return -1;

  connfd = accept(listenfd, NULL, NULL);
  if(connfd < 0)
    {
      fprintf(stderr, "Error: %s\n", strerror(errno));
      return -1;
    }

#ifndef NO_TCPD
  err = getpeername(connfd, (struct sockaddr *)&peer, (socklen_t *) &addrlen);
  if (err < 0)
    {
      close(connfd);
      return -1;
    }

  client = gethostbyaddr((char *)&peer.sin_addr, 
			 sizeof(peer.sin_addr), AF_INET);
  client_hostnum = inet_ntoa(peer.sin_addr);
  if (client_hostnum && !strcmp(client_hostnum, "0.0.0.0"))
    client_machine = "localhost";
  else if (client == NULL)
    client_machine = client_hostnum;
  else
    client_machine = client->h_name;    

  if(hosts_ctl(server_name, client_machine, client_hostnum, 
	       STRING_UNKNOWN) == 0) {
    fprintf(stderr, "WARNING: connection rejected from \"%s\" (%s)\n",
	    client_machine, client_hostnum);
    close(connfd);
    errno = 0;
    return -1;
  }
#endif

  return ant_add_connection(connfd);
}

void 
ant_close_connection(int sock)
{
  ant_connection *c;

  c = lookup_connection(sock);
  if (!c)
    return;

  sock_close_connection(sock);
#ifdef USE_IPC
  if (c->dir == ANT_SERVER)
    IPC_unsubscribeFD(sock, (FD_HANDLER_TYPE)ant_callback_handle_data);
#endif
  free(c->prompt);

  if (c->identity)
    free(c->identity);
  if (c->buffer)
    free(c->buffer);

  connection_num--;  
  conn[c->which] = conn[connection_num];

}

void 
ant_set_default_prompt(char *prompt)
{
  sprintf(default_prompt, "%s", prompt);
}

static void
check_capacity(ant_connection *c, int num_chars_to_add)
{
  char *new_buffer;

  if (c->buffer_size == 0)
    {
      c->buffer = (char *)calloc(1000, sizeof(char)); /* check_alloc checked */
      ant_test_alloc(c->buffer);

      return;
    }

  if (c->num_chars_in_buffer + num_chars_to_add < c->buffer_size)
    return;

  c->buffer_size = c->num_chars_in_buffer + num_chars_to_add + 100;
  new_buffer = realloc(c->buffer, c->buffer_size*sizeof(char)); /* check_alloc checked */
  ant_test_alloc(new_buffer);

  c->buffer = new_buffer;
}

static int
extract_complete_line(ant_connection *c, char **line_buffer)
{
  int ch_index;
  int line_length = 0;

  if (c == NULL || c->buffer == NULL)
    return -1;

  for (ch_index = 0; ch_index < c->num_chars_in_buffer; ch_index++)
    if (c->buffer[ch_index] == '\n')
      {
	line_length = ch_index;
	break;
      }

  if (line_length == 0) {
    c->num_chars_in_buffer--;
    memcpy(c->buffer, c->buffer+1, c->num_chars_in_buffer);
    *line_buffer = NULL;
    return 0;
  }

  *line_buffer = (char *)calloc(line_length+1, sizeof(char)); /* check_alloc checked */
  ant_test_alloc(*line_buffer);
  
  strncpy(*line_buffer, c->buffer, line_length);
  (*line_buffer)[line_length] = '\0';
  c->num_chars_in_buffer -= line_length+1;
  if (c->num_chars_in_buffer)
    memcpy(c->buffer, c->buffer+line_length+1, c->num_chars_in_buffer);

  /* erase any extraneous blanks/carriage returns/etc at the end of the line */
  while (line_length > 0 && isspace((*line_buffer)[line_length-1])) 
    {
      (*line_buffer)[line_length-1] = '\0';
      line_length--;
    }  
  if (line_length == 0)
    {
      free(*line_buffer);
      *line_buffer = NULL;
      return 0;
    }

  return 0;
}

static char *
skip_non_blanks(char *line)
{
  if (!line)
    return line;

  for (; *line; line++)
    if (isspace(*line))
      return line;
    
  return line;
}

static char *
skip_blanks(char *line)
{
  if (!line)
    return line;

  for (; *line; line++)
    if (!isspace(*line))
      return line;
    
  return line;
}

static char *
skip_to_char(char *line, char key_char)
{
  if (!line)
    return line;
    
  for (; *line; line++)
    if (*line == key_char)
      return line+1;
    
  return line;
}

static int
count_tokens_in_line(char *line)
{
  int num_tokens;

  if (!line)
    return 0;

  line = skip_blanks(line);
  num_tokens = 0;
  while (line && *line)
    {
      num_tokens++;
      if (*line == '"' || *line == '\'')
	line = skip_to_char(line+1, *line);
      else
	line = skip_non_blanks(line);      
      line = skip_blanks(line);
    }
  return num_tokens;
}

static void
free_arg_list(int argc, ant_argument **ant_argv)
{
  int arg_index;

  if (ant_argv == NULL || *ant_argv == NULL)
    return;

  for (arg_index = 0; arg_index < argc; arg_index++)
    free((*ant_argv)[arg_index]);

  free(*ant_argv);
  *ant_argv = NULL;
}

static int 
handle_complete_line(ant_connection *c)
{
  int num_tokens;
  char *line_buffer = NULL;
  char *line, *next_token;
  int token_length;
  ant_argument *ant_argv = NULL;
  int arg_index;
  int callback_index;
  int err;

  err = extract_complete_line(c, &line_buffer);
  if (err < 0)
    return -1;
  if (line_buffer == NULL) {
    return echo_prompt(c);
  }

  num_tokens = count_tokens_in_line(line_buffer);
  
  if (num_tokens > 0)
    {
      ant_argv = (ant_argument *)calloc(num_tokens, sizeof(ant_argument)); /* check_alloc checked */
      ant_test_alloc(ant_argv);
      
      line = line_buffer;
      for (arg_index = 0; arg_index < num_tokens; arg_index++)
	{
	  line = skip_blanks(line);
	  if (*line == '"' || *line == '\'')
	    {	  
	      next_token = skip_to_char(line+1, *line);
	      /* Get rid of quote mark at beginning */
	      line++;
	      
	      token_length = next_token - line + 1;
	      /* And get rid of quote mark at end */
	      token_length--;
	    }
	  else
	    {
	      next_token = skip_non_blanks(line);
	      token_length = next_token - line + 1;
	    }
	  
	  ant_argv[arg_index] = 
	    (ant_argument)calloc(token_length+1, sizeof(char)); /* check_alloc checked */
	  ant_test_alloc(ant_argv[arg_index]);
	  
	  strncpy(ant_argv[arg_index], line, token_length-1);
	  ant_argv[arg_index][token_length] = '\0'; 
	  line = next_token;
	} /* for (arg_index = 0; arg_index < num_tokens; arg_index++) */
    } /* if (num_tokens > 0) */

  free(line_buffer);

  if (num_tokens == 0)
    {
      if (null_callback)
	null_callback(c->sock);    
    }
  else 
    {
      for (callback_index = 0; callback_index < callback_num; callback_index++)
	{
	  if (strcasecmp(callback_list[callback_index].command, ant_argv[0]) 
	      == 0)
	    break;
	}

      if (callback_index < callback_num)
	callback_list[callback_index].handler(num_tokens, ant_argv, c->sock);
      else if (default_callback)
	default_callback(num_tokens, ant_argv, c->sock);
    }

  if (num_tokens > 0)
    free_arg_list(num_tokens, &ant_argv);

  /* Check to make sure connection is still active after handling 
     callback */
  c = lookup_connection(c->sock);
  if (c)
    return echo_prompt(c);  
  else
    return 0;
}

static int
check_for_complete_line(ant_connection *c)
{
  int ch_index;

  if (!c || !c->buffer)
    return 0;

  for (ch_index = 0; ch_index < c->num_chars_in_buffer; ch_index++)
    if (c->buffer[ch_index] == '\n')
      return 1;

  return 0;
}

#ifdef USE_READLINE
static void
ant_callback_handle_rl_data(char *line)
{
  int num_read = strlen(line);
  ant_connection *c;
  int line_complete;
  int err;

  add_history(line);

  c = lookup_connection(STDIN_FILENO);
  check_capacity(c, num_read+1);

  strncpy(c->buffer+c->num_chars_in_buffer, line, num_read);

  c->num_chars_in_buffer += num_read;
  c->buffer[c->num_chars_in_buffer] = '\n';
  c->num_chars_in_buffer++;

  line_complete = 0;
  do 
    {
      line_complete = check_for_complete_line(c);
      if (line_complete)
	{
	  err = handle_complete_line(c);
	}
    }
  while (line_complete);

}
#endif

static int 
ant_callback_handle_data(int fd)
{
  int num_to_read, num_read;
  int line_complete;
  int err;
  ant_connection *c;

  c = lookup_connection(fd);

  if (!c)
    return -1;

  num_to_read = ant_bytes_to_read(c->sock);
  if (num_to_read <= 0)
    {
      ant_close_connection(c->sock);
      return -1;
    }

  check_capacity(c, num_to_read);

  num_read = ant_readn(c->sock, c->buffer+c->num_chars_in_buffer, num_to_read, 2.0);
  if (num_read < 0 || num_read < num_to_read)
    {
      ant_close_connection(c->sock);
      return -1;
    }

  if (c->buffer[c->num_chars_in_buffer+num_read-1] == '\0')
    num_read--;

  c->num_chars_in_buffer += num_read;

  line_complete = 0;
  do 
    {
      line_complete = check_for_complete_line(c);
      if (line_complete)
	{
	  err = handle_complete_line(c);
	  if (err < 0)
	    {
	      ant_close_connection(c->sock);
	      return -1;
	    }
	}
    }
  while (line_complete);

  return 0;
}

int ant_register_callback(char *command, 
			  void (*handler)(int, ant_argument *, int))
{
  if (callback_num == MAX_CALLBACKS)
    {
      fprintf(stderr, "Max number of callbacks is %d : callback %s not "
	      "registered.\n", MAX_CALLBACKS, command);
      return -1;
    }
  
  strcpy(callback_list[callback_num].command, command);
  callback_list[callback_num].handler = handler;
  callback_list[callback_num].locked = 1;
  callback_num++;
  return 0;
}

int ant_register_unlocked_callback(char *command, 
				   void (*handler)(int, ant_argument *, int))
{
  fprintf(stderr, "Ant Lite does not support unlocked callbacks. The unlocked "
	  "callback\non %s will be treated just like regular callbacks.\n", 
	  command);

  return ant_register_callback(command, handler);
}

int ant_register_null_callback(void (*handler)(int))
{
  null_callback = handler;
  return 0;
}

int ant_register_default_callback(void (*handler)(int, ant_argument *, int))
{
  default_callback = handler;
  return 0;
}

int 
ant_listen(int listenfd, double timeout, char *server_name)
{
  fd_set readsock, errsock;
  struct timeval t;
  int i;
  int num_ready;
  int max_fd;

  FD_ZERO(&readsock);  
  FD_SET(listenfd, &readsock);
  max_fd = listenfd;

  for (i = 0; i < connection_num; i++)
    {
      if (conn[i].dir == ANT_SERVER)
	{
	  FD_SET(conn[i].sock, &readsock);
	  max_fd = max_fd < conn[i].sock ? conn[i].sock : max_fd;
	}
    }

  FD_ZERO(&errsock);
  FD_SET(listenfd, &errsock);
  for (i = 0; i < connection_num; i++)
    {
      if (conn[i].dir == ANT_SERVER)
	FD_SET(conn[i].sock, &readsock);
    }

  t.tv_sec = (int)timeout;
  t.tv_usec = (int)(timeout*1e6);  

  num_ready = select(max_fd + 1, &readsock, NULL, &errsock, &t);
  if (num_ready < 0)
    return -1;

  if (num_ready == 0)
    return 0;

  if (FD_ISSET(listenfd, &readsock))
    {
      ant_handle_connect(listenfd, server_name);      
    }
  else if (FD_ISSET(listenfd, &errsock))
    {
      return -1;
    }

  for (i = 0; i < connection_num; i++)
    {
      if (conn[i].dir == ANT_SERVER)
	{
	  if (FD_ISSET(conn[i].sock, &readsock))
	    {
#ifdef USE_READLINE
	      if (conn[i].sock == STDIN_FILENO)
		rl_callback_read_char();
	      else
#endif
		ant_callback_handle_data(conn[i].sock);
	    }
	  else if (FD_ISSET(conn[i].sock, &errsock))
	    ant_close_connection(conn[i].sock);
	}
    }

  return 0;
}

int ant_set_identity(int sock, char *identity)
{
  ant_connection *c;
  
  c = lookup_connection(sock);
  if (!c)
    return -1;

  if (c->identity)
    free(c->identity);
  
  if (!identity || strlen(identity) == 0)
    c->identity = NULL;
  else
    {
      c->identity = (char *)calloc(strlen(identity)+1, sizeof(char)); /* check_alloc checked */
      ant_test_alloc(c->identity);
      strcpy(c->identity, identity);
      c->identity[strlen(identity)] = '\0';
    }

  if (c->dir == ANT_CLIENT)
    ant_send_client_command(sock, "identity %s", identity);

  return 0;

}

char *ant_get_identity(int sock)
{
  ant_connection *c;
  
  c = lookup_connection(sock);
  if (!c)
    return NULL;
  
  return c->identity;
}

static int
get_prompt_from_server(int sock, ant_connection *c)
{
  int n;
  int prompt_length;

  do 
    {
      prompt_length = ant_bytes_to_read(sock);
    }
  while (prompt_length == 0);

  c->prompt = (char *)calloc(prompt_length+1, sizeof(char)); /* check_alloc checked */
  ant_test_alloc(c->prompt);

  n = ant_readn(sock, c->prompt, prompt_length, -1);
  if (n < 0)
    return -1;
  c->prompt[prompt_length] = '\0';

  return 0;
}

int 
ant_readline(int sock, char *buffer, int length)
{  
  int n, i;
  char c;

  i = 0;
  do {
    n = ant_readn(sock, &c, 1, -1);
    if(n == 1 && i < length) {
      buffer[i] = c;
      i++;
    }
  } while(n == 1 && c != '\n');
  if (i < length) {
    i--;
    buffer[i] = '\0';
  }
  return i;
}

static int 
ant_read_prompt(int sock, char *data, int length)
{  
  int n = 0;
  char *expected_prompt;
  char *prompt;
  int num_to_read = 0;
  int prompt_length = 0;
  ant_connection *c;
  int collected_data_length = 0;

  c = lookup_connection(sock);
  if (!c)
    return -1;
  
  expected_prompt = c->prompt;
  if (!expected_prompt || strlen(expected_prompt) == 0)
    return get_prompt_from_server(sock, c);

  prompt_length = strlen(expected_prompt);
  prompt = (char *)calloc(prompt_length+1, sizeof(char)); /* check_alloc checked */
  ant_test_alloc(prompt);

  num_to_read = ant_bytes_to_read(sock);
  if (num_to_read > prompt_length)
    num_to_read = prompt_length;
  n = ant_readn(sock, prompt, num_to_read, -1);      

  while(strncmp(prompt, expected_prompt, prompt_length))
    {
      if ((signed int)strlen(prompt) == prompt_length)
	{
	  if (data && collected_data_length < length-1)
	    {
	      data[collected_data_length++] = prompt[0];
	      data[collected_data_length] = '\0';
	    }
	  memcpy(prompt, prompt+1, sizeof(char)*prompt_length);
	  prompt[prompt_length-1] = '\0';
	}

      n = ant_readn(sock, prompt+strlen(prompt), 1, -1);
      if(n < 0)
	{
	  free(prompt);
	  return -1;
	}
    }

  free(prompt);
  return 0;
}

int 
ant_send_client_command_with_response(int sock, char *response, int length, char *fmt, ...)
{
  va_list args;
  char command[256];
  int err;
  int n;  

  va_start(args, fmt);
  n = vsnprintf(command, 254, fmt, args);
  va_end(args);

  if (n > -1 && n < 254) {    
    command[n] = '\n';
    command[n+1] = '\0';

    err = ant_writen(sock, command, strlen(command) + 1, -1);
    if(err < 0)
      return -1;     

    err = ant_read_prompt(sock, response, length);
    if(err < 0)
      return -1;
  }
  return 0;
}

int 
ant_send_client_command(int sock, char *fmt, ...)
{
  va_list args;
  char command[256];
  int n;

  va_start(args, fmt);
  n = vsnprintf(command, 254, fmt, args);
  va_end(args);

  if (n > -1 && n < 254) {    
    command[n] = '\n';
    command[n+1] = '\0';
  }

  return ant_send_client_command_with_response(sock, NULL, 0, command);
}

int 
ant_connect_to_server(char *host, int port)
{
  int sockfd;
  int err;

  if(connection_num == MAX_CONNECTIONS)
    return -1;

  sockfd = sock_connect_to_server(host, port);
  if(sockfd < 0)
    return -1;
  conn[connection_num].sock = sockfd;
  conn[connection_num].dir = ANT_CLIENT;
  conn[connection_num].which = connection_num;

  conn[connection_num].buffer = NULL;
  conn[connection_num].identity = NULL;

  conn[connection_num].prompt = (char *)calloc(2, sizeof(char)); /* check_alloc checked */
  ant_test_alloc(conn[connection_num].prompt);

  connection_num++;

  err = ant_read_prompt(sockfd, NULL, 0);
  if(err < 0)
    {
      ant_close_connection(sockfd);
      return -1;
    }

  return sockfd;
}

void 
ant_print_command(FILE *fp, char *str, int ant_argc, ant_argument *ant_argv)
{
  int i;

  fprintf(fp, "%s", str);
  for(i = 0; i < ant_argc; i++)
    fprintf(fp, "%s ", ant_argv[i]);
  fprintf(fp, "\n");
}
