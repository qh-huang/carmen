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

#ifndef SOCKUTILS_H
#define SOCKUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

int sock_writen(int fd, const void *vptr, int n, int timeout);

int sock_readn(int fd, void *vptr, int n, int timeout);

int sock_write_string(int sock, char *s);

int sock_printf(int sock, char *fmt, ...) __attribute__ 
((format (printf, 2, 3)));
int sock_bytes_to_read(int sock);

int sock_connect_to_server(char *host, int port);

void sock_close_connection(int sock);

#ifdef __cplusplus
}
#endif

#endif
