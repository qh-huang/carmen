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

typedef struct sockaddr SA;

int sock_writen(int fd, const void *vptr, int n, int timeout)
{
  size_t nleft;
  ssize_t nwritten;
  const char *ptr;
  fd_set writesock, errsock;
  struct timeval t;
  int result;

  ptr = vptr;
  nleft = (size_t)n;
  while(nleft > 0) {
    FD_ZERO(&writesock);
    FD_ZERO(&errsock);
    FD_SET(fd, &writesock);
    FD_SET(fd, &errsock);
    if(timeout == -1) {
      if(select(fd + 1, NULL, &writesock, &errsock, NULL) < 0) {
	if(errno == EINTR)
	  nwritten = 0;
	else
	  return -1;
      }
    }
    else {
      t.tv_sec = timeout / 1000000;
      t.tv_usec = timeout % 1000000;
      result = select(fd + 1, NULL, &writesock, &errsock, &t);
      if(result < 0)
	if(errno == EINTR)
	  nwritten = 0;
	else
	  return -1;
      else if(result == 0)
	return n - nleft;
    }
    if(FD_ISSET(fd, &errsock)) {
      return -1;
    }
    if((nwritten = write(fd, ptr, nleft)) <= 0) {
      if(errno == EINTR)
	nwritten = 0;
      else
	return -1;
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  return n;
}

int sock_readn(int fd, void *vptr, int n, int timeout)
{
  size_t nleft;
  ssize_t nread;
  char *ptr;
  fd_set readsock, errsock;
  struct timeval t;
  int result;

  ptr = vptr;
  nleft = (size_t)n;
  while(nleft > 0) {
    FD_ZERO(&readsock);
    FD_ZERO(&errsock);
    FD_SET(fd, &readsock);
    FD_SET(fd, &errsock);
    if(timeout == -1) {
      if(select(fd + 1, &readsock, NULL, &errsock, NULL) < 0) {
	if(errno == EINTR)
	  nread = 0;
	else
	  return -1;
      }
    }
    else {
      t.tv_sec = timeout / 1000000;
      t.tv_usec = timeout % 1000000;
      result = select(fd + 1, &readsock, NULL, &errsock, &t);
      if(result < 0) {
	if(errno == EINTR)
	  nread = 0;
	else
	  return -1;
      }
      else if(result == 0)
	return n - nleft;
    }
    if(FD_ISSET(fd, &errsock))
      return -1;
    if((nread = read(fd, ptr, nleft)) < 0) {
      if(errno == EINTR)
	nread = 0;
      else
	return -1;
    } 
    else if(nread == 0)
      return -1;
    nleft -= nread;
    ptr += nread;
  }
  return n;
}

int sock_write_string(int sock, char *s)
{
  int n;
  char c;
  int return_val;
  int length;
  
  length = strlen(s);

  while (length > 0 && (s[length-1] == '\r' || s[length-1] == '\n'))
    length--;

  n = sock_writen(sock, s, length, -1);      
  if (n < 0)
    return n;

  c = '\r';
  return_val = sock_writen(sock, &c, 1, -1);
  if (return_val < 0)
    return return_val;  
      
  c = '\n';
  return_val = sock_writen(sock, &c, 1, -1);
  if (return_val < 0)
    return return_val;  
  
  return n;
}

int sock_printf(int sock, char *fmt, ...) 
{
  va_list args;
  char Buffer[4096];
  int n;

  va_start(args, fmt);
  n = vsnprintf(Buffer, 4096, fmt, args);
  va_end(args);
  if (n > -1 && n < 4096) {    
    return sock_write_string(sock, Buffer);
  }
  return -1;
}

int sock_bytes_to_read(int sock)
{
  int result;
  int err;

  err = ioctl(sock, FIONREAD, &result);
  if (err < 0)
    return err;

  return result;
}

int sock_connect_to_server(char *host, int port)
{
  int sockfd;
  struct hostent *addr;
  unsigned long addr_tmp;
  struct sockaddr_in servaddr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0)
    return -1;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  if(atoi(host) > 0)
    servaddr.sin_addr.s_addr=inet_addr(host);
  else {
    if((addr = gethostbyname(host))==NULL)
      return -1;
    bcopy(addr->h_addr, (char *)&addr_tmp, addr->h_length);
    servaddr.sin_addr.s_addr = addr_tmp;
  }
  servaddr.sin_port = htons(port);
  if(connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
    close(sockfd);
    return -1;
  }
  return sockfd;
}

void sock_close_connection(int sock)
{
  if(sock != -1)
    close(sock);
  sock = -1;
}
