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

#include <carmen/carmen.h>
#include "ant.h"

#define       SERVER_PORT          3000

void ant_command_handler(char *line);
int ant_handle_connect(int listenfd, char *server_name);
int ant_listen_socket;

void test1_handler(int ant_argc, ant_argument *ant_argv, int sock)
{
  printf("start test1 callback.\n");
  ant_print_command(stderr, "COMMAND: ", ant_argc, ant_argv);

  ant_write_string(sock, "Have a nice day.\n");
  printf("end test1 callback.\n");
}

void test2_handler(int ant_argc, ant_argument *ant_argv, int sock __attribute__ ((unused)))
{
  printf("start test2 callback.\n");
  ant_print_command(stderr, "COMMAND: ", ant_argc, ant_argv);
  printf("end test2 callback.\n");
}

void default_handler(int ant_argc, ant_argument *ant_argv, int sock)
{
  fprintf(stderr, "%d ", ant_argc);
  ant_print_command(stderr, "Command: ", ant_argc, ant_argv);
  ant_write_string(sock, "That is not a valid command.\n");
}

void null_handler(int sock)
{
  ant_printf(sock, "The NULL handler. %d\n", 42);
}

int main(int argc, char **argv)
{
  int port = SERVER_PORT;

  if(argc == 2)
    port = atoi(argv[1]);
  
  ant_listen_socket = ant_open_server(port, "TEST");
  if (ant_listen_socket < 0)
    carmen_die("Error: %s\n", strerror(errno));

  ant_set_default_prompt("Tester: ");
  ant_register_unlocked_callback("TEST1", test1_handler);
  ant_register_callback("TEST2", test2_handler);
  ant_register_default_callback(default_handler);
  ant_register_null_callback(null_handler);

  ant_add_connection(STDIN_FILENO);

  while (1)
    {
      ant_listen(ant_listen_socket, 0.1, "TEST");
    }

  return 0;
}

