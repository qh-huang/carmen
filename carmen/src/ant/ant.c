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
#include <pthread.h>
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
#include <pthread.h>

#include "sockutils.h"
#include "ant.h"

#define          MAX_CALLBACKS          100
#define          MAX_CONNECTIONS        10
#define          MAX_TIMERS             100
#define          MAX_HISTORY            10


#ifndef NO_TCPD
#include <tcpd.h>

int hosts_ctl(char *, char *, char *, char *);
int isblank(int c);

int allow_severity = 5;
int deny_severity = 5;
#endif

typedef struct {
  int sock;
  int which;
  int active;
  char identity[100];
  char ant_prompt[100];
  char history[MAX_HISTORY][200];
  int history_num;
  pthread_t thd;
} connection;

typedef struct sockaddr SA;

typedef void (*handler_t)(int, ant_argument *, int);

typedef struct {
  char command[100];
  int locked;
  void (*handler)(int, ant_argument *, int);
} callback;

typedef struct {
  double lastcalled, interval;
  int timerval;
  void (*timerfunc)(void);
} timer;

callback callback_list[MAX_CALLBACKS];
int callback_num = 0;
callback default_callback;
int use_default = 0;
callback null_callback;
int use_null = 0;
timer timer_list[MAX_TIMERS];
int timer_num = 0;

int telnet_command_size = 6;
char telnet_string[6] = {255, 251, 1, 255, 251, 3};

connection conn[MAX_CONNECTIONS];
int connection_num = 0;

char default_prompt[100];
int ant_server_quit = 0;
pthread_mutex_t command_mutex;
int server_loop_running = 0;
pthread_t main_loop_thd;
char server_name_global[100];

double ant_get_time_ms(void)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return tv.tv_sec+tv.tv_usec/1000000.0;
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
    for(i = 0; i < MAX_CONNECTIONS; i++)
      if(conn[i].active)
	sock_write_string(conn[i].sock, s);
    return 0;
  }
  else {
    n = sock_write_string(sock, s);
    return n;
  }
}

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

void ant_get_prompt(int sock, char *prompt)
{
  int i, n;
  
  ant_readn(sock, prompt, 1, -1);
  i = 1;
  do {
    n = ant_readn(sock, prompt + i, 1, 0);
    if(n == 1)
      i++;
  } while(n == 1);
  prompt[i] = '\0';
}

int ant_read_prompt(int sock)
{  
  int n, i;
  char *expected_prompt;
  char prompt[100];

  for(i = 0; i < connection_num; i++) 
    if(conn[i].sock == sock)
      break;

  if(i == connection_num)
    return -1;

  expected_prompt = conn[i].ant_prompt;
  if(strlen(expected_prompt) == 0)
    ant_get_prompt(sock, conn[i].ant_prompt);
  else
    do {
      for(i = 0; i < (signed int)strlen(expected_prompt); i++)
	prompt[i] = prompt[i + 1];
      n = ant_readn(sock, prompt+strlen(expected_prompt) - 1, 1, -1);
      if(n != 1)
	return -1;
    } while(strncmp(prompt, expected_prompt, strlen(expected_prompt)));
  return 0;
}

int ant_send_client_command(int sock, char *fmt, ...)
{
  va_list args;
  char command[256];
  int err;
  int n;

  va_start(args, fmt);
  n = vsnprintf(command, 254, fmt, args);
  va_end(args);

  /* This is almost certainly wrong -- it should be command[n] == '\n'. 
     But it seems to work, so I'm not going to mess with it. NR */

  if (n > -1 && n < 254) {    
    command[n] = '\r';
    command[n+1] = '\0';

    err = ant_read_prompt(sock);
    if(err < 0)
      return -1;

    err = ant_writen(sock, command, strlen(command) + 1, -1);
    if(err < 0)
      return -1; 
    
    ant_readn(sock, command, strlen(command) + 3, -1);
  }
  return -1;
}

int ant_open_server(int port, char *serverName __attribute__ ((unused)))
{
  int listenfd, err;
  struct sockaddr_in servaddr;
  int arg = 1;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenfd < 0)
    return -1;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)&arg, sizeof(arg));
  err = bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
  if(err < 0)
    return -1;
  return listenfd;
}

static int listen_for_connection(int listenfd, char *server_name)
{
  int connfd;
  fd_set readsock, errsock;
  struct timeval t;

#ifndef NO_TCPD
  struct sockaddr_in peer;
  int addrlen=sizeof(peer);
  struct hostent *client;
  char *client_machine, *client_hostnum;
#endif

  server_name = server_name;

  listen(listenfd, 5);
  FD_ZERO(&readsock);
  FD_SET(listenfd, &readsock);
  FD_ZERO(&errsock);
  FD_SET(listenfd, &errsock);
  t.tv_sec = 0;
  t.tv_usec = 0;
  if(select(listenfd + 1, &readsock, NULL, &errsock, &t) <= 0)
    return -1;

  /* Patched by Nick Roy, 2/28/02 
     Was connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
     except that clilen was never initialised, had the wrong value, 
     and cliaddr and clilen were never used afterwards. */

  connfd = accept(listenfd, NULL, NULL);
  if(connfd < 0)
    return -1;

#ifndef NO_TCPD
  getpeername(connfd, (struct sockaddr *)&peer, (socklen_t *) &addrlen);
  client = gethostbyaddr((char *)&peer.sin_addr, 
			 sizeof(peer.sin_addr), AF_INET);
  client_hostnum = inet_ntoa(peer.sin_addr);
  if (!strcmp(client_hostnum, "0.0.0.0"))
    client_machine = "localhost";
  else if (client == 0)
    client_machine = client_hostnum;
  else
    client_machine = client->h_name;

  if(hosts_ctl(server_name, client_machine, client_hostnum, 
	       STRING_UNKNOWN) == 0) {
    fprintf(stderr, "WARNING: connection rejected from \"%s\" (%s)\n",
	    client_machine, client_hostnum);
    close(connfd);
    return -1;
  }
  else
#endif
    return connfd;
}

void ant_close_connection(int sock)
{
  int i;

  if(sock != -1)
    for(i = 0; i < connection_num; i++)
      if(conn[i].active && conn[i].sock == sock) {
	conn[i].active = 0;
	connection_num--;
      }
  sock_close_connection(sock);
}

int ant_connect_to_server(char *host, int port)
{
  int sockfd;
  char junk[100];

  if(connection_num == MAX_CONNECTIONS)
    return -1;

  sockfd = sock_connect_to_server(host, port);
  if(sockfd < 0)
    return -1;
  conn[connection_num].sock = sockfd;
  conn[connection_num].which = connection_num;
  conn[connection_num].active = 1;
  strcpy(conn[connection_num].ant_prompt, "");
  connection_num++;

  ant_readn(sockfd, junk, telnet_command_size, -1);
  ant_writen(sockfd, junk, telnet_command_size, -1);
  return sockfd;
}

void ant_set_default_prompt(char *prompt)
{
  sprintf(default_prompt, "%s", prompt);
}

static void prompt_handler(int ant_argc, ant_argument *ant_argv, int sock) 
{
  char Prompt[255];
  int length;
  int conn_index;

  if (ant_argc < 2)
    return;

  length = strlen(ant_argv[1]);
  if (length > 250)
    return;

  strcpy (Prompt, ant_argv[1]);
  if (Prompt[length-2] == '\\' &&
      Prompt[length-1] == 'N') {
    Prompt[length-2] = '\n';
    Prompt[length-1] = '\0';
  }

  for (conn_index = 0; conn_index < connection_num; conn_index++) {
    if (conn[conn_index].sock == sock) {
      strcpy(conn[conn_index].ant_prompt, Prompt); 
      break;
    }
  }
}

int ant_readline(int sock, char *buffer, int length)
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

void add_to_history(connection *c, int ant_argc, ant_argument *ant_argv)
{
  char str[200];
  int i;

  strcpy(str, ant_argv[0]);
  for(i = 1; i < ant_argc; i++)
    sprintf(str, "%s %s", str, ant_argv[i]);
  if(c->history_num < MAX_HISTORY) {
    strcpy(c->history[c->history_num], str);
    c->history_num++;
  }
  else {
    for(i = 0; i < MAX_HISTORY - 1; i++)
      strcpy(c->history[i], c->history[i + 1]);
    strcpy(c->history[MAX_HISTORY - 1], str);
  }
}

static int get_command(connection *c, int timeout, int prompt, char *ant_prompt, 
		char *text_command, int *command_length,
		int *argc, ant_argument argv[10])
{
  int n, i, done, err;
  char response[200], cmd[10];
  int mark;
  char *mark2, *mark3, tempc;
  int length;
  int history_mark = c->history_num;

  int ctl_key, backspace, clear, up, down, tab;

  if(prompt) {
    //    fprintf(stderr, "wrote prompt\n");
    n = ant_writen(c->sock, ant_prompt, strlen(ant_prompt), timeout);
    if(n != (signed int) strlen(ant_prompt))
      return -1;
  }

  mark = *command_length;
  strncpy(response, text_command, *command_length);

  do {
    if((n = ant_readn(c->sock, &tempc, 1, timeout)) == 1) {
      response[mark] = tempc;
      //      fprintf(stderr, "received a %d %c\n", tempc, tempc);
      ctl_key = 0;
      backspace = 0;
      clear = 0;
      up = 0;
      down = 0;
      tab = 0;
      if(tempc == 27) {                     // control keys
	ctl_key = 1;
	ant_readn(c->sock, &tempc, 1, timeout);
	ant_readn(c->sock, &tempc, 1, timeout);
	if(tempc == 65)
	  up = 1;
	else if(tempc == 66)
	  down = 1;
	if(tempc >= 49 && tempc <= 54) {
	  ant_readn(c->sock, &tempc, 1, timeout);
	  if(tempc == 126)
	    backspace = 1;
	}
      }
      else if(tempc == 8 || tempc == 127) {
	ctl_key = 1;
	backspace = 1;
      }
      else if(tempc == 21) {
	ctl_key = 1;
	clear = 1;
      }
      else if(tempc == '\t') {
	ctl_key = 1;
	tab = 1;
      }
      else if((!isgraph(tempc) && tempc != '\r' && tempc != '\n' && 
	       tempc != '\0' && tempc != ' ') || tempc == '\t')
	ctl_key = 1;
      
      cmd[0] = 8;
      cmd[1] = ' ';
      cmd[2] = 8;
      if(backspace && mark > 0) {
	err = ant_writen(c->sock, cmd, 3, timeout);
	if(err < 0)
	  return -1;
	mark--;
      }
      else if(clear && mark > 0) {
	while(mark > 0) {
	  err = ant_writen(c->sock, cmd, 3, timeout);
	  if(err < 0)
	    return -1;
	  mark--;
	}
      }
      else if(up || down) {
	while(mark > 0) {
	  err = ant_writen(c->sock, cmd, 3, timeout);
	  if(err < 0)
	    return -1;
	  mark--;
	}
	if(down) {
	  history_mark++;
	  if(history_mark > c->history_num)
	    history_mark = c->history_num;
	}
	else if(up) {
	  history_mark--;
	  if(history_mark < 0)
	    history_mark = 0;
	}
	if(history_mark != c->history_num) {
	  err = ant_writen(c->sock, c->history[history_mark],
		     strlen(c->history[history_mark]), timeout);
	  if(err < 0)
	    return -1;
	  strcpy(response, c->history[history_mark]);
	  mark = strlen(c->history[history_mark]);
	}
      }
      else if(!ctl_key) {
	err = ant_writen(c->sock, &response[mark], 1, timeout);
	if(err < 0)
	  return -1;
	mark++;
      }
    }
  } while(n == 1 && mark < 200 && (tempc != '\0' && tempc != '\n'));

  *command_length = mark;
  strncpy(text_command, response, *command_length);

  if(n == 0)
    return 0;  
  if(n == -1 || mark == 200)
    return -1;

  /* Was cmd[1] = '\r', cmd[0] = '\n'. Patched by Nick Roy */

  cmd[0] = '\r';
  cmd[1] = '\n';
  err = ant_writen(c->sock, cmd, 2, timeout);
  if(err < 0)
    return -1;

  response[mark - 2] = '\0';

  mark2 = response;
  while(mark2[0] == ' ')
    mark2++;
  mark3 = mark2;

  *argc = 0;
  done = 0;
  do {
    mark2 = mark3;
    mark3 = strchr(mark2, ' ');
    if(mark3 != NULL)
      mark3++;
    if(mark3 == NULL) {
      strcpy(argv[*argc], mark2);
      length = strlen(argv[*argc]);
      if (argv[*argc][0] != '\"' || argv[*argc][length-1] != '\"') {
	for (i = 0; i < length; i++)
	  argv[*argc][i] = toupper(argv[*argc][i]);
      }
      done = 1;
      if(mark2[0] != ' ' && strlen(argv[*argc]) > 0)
	(*argc)++;
    }
    else {
      strncpy(argv[*argc], mark2, mark3 - mark2 - 1);
      argv[*argc][mark3 - mark2 - 1] = '\0';
      length = strlen(argv[*argc]);
      if (argv[*argc][0] != '\"' || argv[*argc][length-1] != '\"') {
	for (i = 0; i < length; i++)
	  argv[*argc][i] = toupper(argv[*argc][i]);
      }
      if(mark2[0] != ' ')
	(*argc)++;
    }
  } while(!done);
  
  *command_length = 0;
  text_command[0] = '\0';

  return 1;
}

int ant_register_callback(char *command, 
			   void (*handler)(int, ant_argument *, int))
{
  strcpy(callback_list[callback_num].command, command);
  callback_list[callback_num].handler = handler;
  callback_list[callback_num].locked = 1;
  callback_num++;
  return 0;
}

int ant_register_unlocked_callback(char *command, 
				   void (*handler)(int, ant_argument *, int))
{
  strcpy(callback_list[callback_num].command, command);
  callback_list[callback_num].handler = handler;
  callback_list[callback_num].locked = 0;
  callback_num++;
  return 0;
}

int ant_register_null_callback(void (*handler)(int))
{
  null_callback.handler = (handler_t)handler;
  use_null = 1;
  return 0;
}

int ant_register_default_callback(void (*handler)(int, ant_argument *, int))
{
  default_callback.handler = handler;
  use_default = 1;
  return 0;
}

int ant_register_timer(int timerval, void (*timerfunc)(void))
{
  timer_list[timer_num].timerval = timerval;
  timer_list[timer_num].timerfunc = timerfunc;
  timer_num++;
  return 0;
}

int ant_set_identity(int sock, char *identity)
{
  int i;
  
  for(i = 0; i < MAX_CONNECTIONS; i++)
    if(conn[i].active && conn[i].sock == sock) {
      strcpy(conn[i].identity, identity);
      return 0;
    }
  return -1;
}

char *ant_get_identity(int sock)
{
  int i;
  
  for(i = 0; i < MAX_CONNECTIONS; i++)
    if(conn[i].active && conn[i].sock == sock)
      return conn[i].identity;
  return NULL;
}

static void send_telnet_commands(int sock)
{
  unsigned char response[20];

  ant_writen(sock, telnet_string, telnet_command_size, -1);
  ant_readn(sock, response, telnet_command_size, -1);
}

static void *command_loop(void *x)
{
  connection *c = (connection *)x;
  int done, err, ant_argc;
  ant_argument ant_argv[10];
  int i, prompt = 1, matched_callback;
  int command_length;
  char text_command[200];

  for (i = 0; i < 10; i++)
    ant_argv[i] = (char *)calloc(40, sizeof(char)); /* check_alloc checked */

  send_telnet_commands(c->sock);
  done = 0;
  command_length = 0;
  while(!done) {
    err = get_command(c, 1000000, prompt, c->ant_prompt, text_command,
		      &command_length, &ant_argc, ant_argv);
    if(err < 0)
      done = 1;
    else if(err == 0)
      prompt = 0;
    else if(err == 1 && ant_argc == 0) {
      prompt = 1;
      if(use_null) {
	pthread_mutex_lock(&command_mutex);
	null_callback.handler(ant_argc, ant_argv, c->sock);
	pthread_mutex_unlock(&command_mutex);
      }
    }
    else if(err == 1 && ant_argc > 0) {
      prompt = 1;
      if(strncmp(ant_argv[0], "QUIT", 4) == 0)
	done = 1;
      if(!done) {
	matched_callback = 0;
	for(i = 0; i < callback_num; i++) {
	  if(strcmp(ant_argv[0], callback_list[i].command) == 0) {
	    matched_callback = 1;
	    if(callback_list[i].locked)
	      pthread_mutex_lock(&command_mutex);
	    callback_list[i].handler(ant_argc, ant_argv, c->sock);
	    if(callback_list[i].locked)
	      pthread_mutex_unlock(&command_mutex);
	  }
	}
	if(matched_callback)
	  add_to_history(c, ant_argc, ant_argv);

	if(!matched_callback  && use_default) {
	  pthread_mutex_lock(&command_mutex);
	  default_callback.handler(ant_argc, ant_argv, c->sock);
	  pthread_mutex_unlock(&command_mutex);
	}
      }
    }
  }
  ant_close_connection(c->sock);
  return NULL;
}

int ant_server_main_loop(char *server_name, int port)
{
  int listen_sock, server_sock = -1, i;
  double min_interval;
  sigset_t set;

  ant_register_callback("PROMPT", prompt_handler);
  
  server_loop_running = 1;
  pthread_mutex_init(&command_mutex, NULL);
  printf("Opening ANT server on port %d.\n", port);
  listen_sock = ant_open_server(port, server_name);
  if(listen_sock == -1) {
    printf("Error: Could not open server.\n");
    return -1;
  }

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGPIPE);
  sigprocmask(SIG_BLOCK, &set, NULL);

  for(i = 0; i < timer_num; i++)
    timer_list[i].lastcalled = ant_get_time_ms();
  for(i = 0; i < MAX_CONNECTIONS; i++)
    conn[i].active = 0;
  while(!ant_server_quit) {
    server_sock = listen_for_connection(listen_sock, server_name);

    if(server_sock != -1) {
      if(connection_num == MAX_CONNECTIONS) {
	ant_write_string(server_sock, 
			 "Maximum number of connections reached.\n");
	ant_close_connection(server_sock);
      }
      else {
	i = 0;
	while(conn[i].active) {
	  i++;
	}
	conn[i].sock = server_sock;
	conn[i].which = i;
	conn[i].active = 1;
	conn[i].history_num = 0;
	strcpy(conn[i].identity, "DEFAULT");
	strcpy(conn[i].ant_prompt, default_prompt);
	
	/* Was pthread_create, then connection_num. This created a race
  	   condition that broke Sphinx on 2.4.16 kernels. Patched 2/13/02,
  	   Nick Roy */

	connection_num++;
	pthread_create(&(conn[i].thd), NULL, command_loop, (void *)&(conn[i]));
      }
    }
    min_interval = 2;
    for(i = 0; i < timer_num; i++) {
      timer_list[i].interval = ant_get_time_ms() - timer_list[i].lastcalled;
      if(timer_list[i].interval > timer_list[i].timerval / 1000000.0) {
	pthread_mutex_lock(&command_mutex);
	timer_list[i].timerfunc();
	pthread_mutex_unlock(&command_mutex);
	timer_list[i].lastcalled = ant_get_time_ms(); 
      }
      timer_list[i].interval = ant_get_time_ms() - timer_list[i].lastcalled;
      if(timer_list[i].timerval / 1000000.0 - timer_list[i].interval < 
	 min_interval)
	min_interval = timer_list[i].timerval / 1000000.0 - 
	  timer_list[i].interval;
    }
    if(timer_num == 0)
      usleep(100000);
    else if(min_interval > 0) {
      usleep((int)(min_interval * 100000));
    }
  }
  ant_close_connection(server_sock);
  ant_close_connection(listen_sock);
  pthread_mutex_destroy(&command_mutex);
  server_loop_running = 0;
  pthread_exit(0);
  return 0;
}

static void *main_loop_helper(void *x)
{
  int port = *(int *)x;

  if(!server_loop_running)
    ant_server_main_loop(server_name_global, port);
  return NULL;
}

void ant_server_main_loop_threaded(char *server_name, int port)
{
  int *x = calloc(1, sizeof(int)); /* check_alloc checked */
  if (x == NULL) {
    fprintf(stderr, "Out of memory in %s (%s, line %d).\n", __FUNCTION__,
	    __FILE__, __LINE__);
    exit(-1);
  }

  strcpy(server_name_global, server_name);
  *x = port;
  pthread_create(&main_loop_thd, NULL, main_loop_helper, x);
}

void ant_shutdown(int fd __attribute__ ((unused)))
{
  ant_server_quit = 1;
}

int ant_running(void)
{
  return !ant_server_quit;
}

void ant_print_command(FILE *fp, char *str, int ant_argc, ant_argument *ant_argv)
{
  int i;

  fprintf(fp, "%s", str);
  for(i = 0; i < ant_argc; i++)
    fprintf(fp, "%s ", ant_argv[i]);
  fprintf(fp, "\n");
}
