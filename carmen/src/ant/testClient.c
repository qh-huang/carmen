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
#include <unistd.h>
#include <string.h>
#include "ant.h"

#define       DEFAULT_PORT        3000

static char *
get_from_bin_host(void) {
  FILE *bin_host;
  char hostname[255];

  if (getenv("HOST") == NULL) 
    {
      if (getenv("HOSTNAME") != NULL)       
	setenv("HOST", getenv("HOSTNAME"), 1);
      else if (getenv("host") != NULL)       
	setenv("HOST", getenv("host"), 1);
      else if (getenv("hostname") != NULL)       
	setenv("HOST", getenv("hostname"), 1);
      else 
	{
	  bin_host = popen("/bin/hostname", "r");
	  if (bin_host == NULL)
	    return NULL;
	  fscanf(bin_host, "%s", hostname);
	  setenv("HOST", hostname, 1);
	}
    }
  return getenv("HOST");
}

int main(int argc, char **argv)
{
  char host[100];
  int port, client_sock;
  char buf[1024];

  strcpy(host, get_from_bin_host());
  port = DEFAULT_PORT;
  if(argc >= 2)
    strcpy(host, argv[1]);
  if(argc >= 3)
    port = atoi(argv[2]);
  
  client_sock = -1;
  while(client_sock < 0) {
    client_sock = ant_connect_to_server(host, port);
    if(client_sock < 0) {
      fprintf(stderr, "Could not connect to server %s:%d\n", host, port);
      sleep(1);
    }
  }
  
  buf[0] = '\0';
  ant_send_client_command_with_response(client_sock, buf, 100, "test1");
  if (strlen(buf) > 0)
    fprintf(stderr, "Received: %s\n", buf);

  ant_send_client_command_with_response(client_sock, buf, 1024, "help");
  if (strlen(buf) > 0)
    fprintf(stderr, "Received: %s\n", buf);

  ant_send_client_command(client_sock, "quit");

  return 0;
}
