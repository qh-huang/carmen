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

#ifndef ANT_H
#define ANT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef char *ant_argument;

/* server / client commands */

void ant_set_default_prompt(char *prompt);

int ant_writen(int fd, const void *vptr, int n, int timeout);

int ant_readn(int fd, void *vptr, int n, int timeout);

int ant_write_string(int sock, char *s);

int ant_printf(int sock, char *fmt, ...) __attribute__ 
((format (printf, 2, 3)));

int ant_set_identity(int sock, char *identity);

char *ant_get_identity(int sock);

/* server only commands */

int ant_open_server(int port, char *serverName);

int ant_register_callback(char *command,
			   void (*handler)(int, ant_argument *, int));

int ant_register_default_callback(void (*handler)(int, ant_argument *, int));

int ant_register_null_callback(void (*handler)(int));

int ant_add_connection(int connfd);

void ant_shutdown(int listen_socket);

int ant_handle_connect(int listenfd, char *server_name);

void ant_print_command(FILE *fp, char *str, int ant_argc, ant_argument *ant_argv);

int ant_listen(int listenfd, double timeout, char *server_name);

/* client only commands */

int ant_connect_to_server(char *host, int port);

void ant_close_connection(int sock);

int ant_send_client_command(int sock, char *fmt, ...)  __attribute__ 
((format (printf, 2, 3)));

int ant_send_client_command_with_response(int sock, char *response, int length, char *fmt, ...)  __attribute__ 
((format (printf, 4, 5)));

int ant_readline(int sock, char *buffer, int length);

/* old ant commands. these will probably break. */

int ant_register_unlocked_callback(char *command, 
				   void (*handler)(int, ant_argument *, int));

int ant_register_timer(int timerval, void (*timerfunc)(void));

int ant_server_main_loop(char *server_name, int port);

void ant_server_main_loop_threaded(char *server_name, int port);

int ant_running(void);

#ifdef __cplusplus
}
#endif

#endif
