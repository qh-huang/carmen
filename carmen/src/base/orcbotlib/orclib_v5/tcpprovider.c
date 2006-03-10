#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
//#include <wait.h>
#include <ctype.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include "orc.h"

typedef struct 
{
	char *hostname;
	int port;
} tcpprovider_t;

static int tcp_connect(orc_comms_impl_t *impl)
{
	tcpprovider_t *t = (tcpprovider_t*) impl->_p;

	struct hostent *host;
	struct sockaddr_in sa;
	int thesocket;

	/* let's find out about this host */
	host = gethostbyname(t->hostname);
	if (host==NULL)
	{
		//      perror(hostname);
		return -1;
	}

	/* create the socket */
	thesocket=socket(AF_INET,SOCK_STREAM,0);

	/* fill in the fields */
	bzero(&sa,sizeof(sa));
	sa.sin_family=AF_INET;
	sa.sin_port=htons(0);
	sa.sin_addr.s_addr=htonl(INADDR_ANY);

	/* bind it to the port */
	if (bind (thesocket, (struct sockaddr *) &sa, sizeof (sa)) <0)
	{
		close(thesocket);
		return -1;
	}

	sa.sin_port=htons(t->port);
	sa.sin_addr=*(struct in_addr *) host->h_addr;

	if (connect(thesocket, (struct sockaddr *) &sa, sizeof (sa)))
	{
		if (errno!=EINPROGRESS)
		{
			close(thesocket);
			return -1;
		}
	}

	// prevent "broken pipe" signals.
	signal(SIGPIPE, SIG_IGN);

	// disable nagle algorithm
	int n=1;
	if (setsockopt (thesocket, IPPROTO_TCP, TCP_NODELAY, 
			(char *) &n, sizeof(n))<0)
	{
		perror("could not setsockopt");
		close(thesocket);
		return -1;
	}

	return thesocket;
}

static void tcp_disconnect(orc_comms_impl_t *impl __attribute__ ((unused)), int fd)
{
	close(fd);
	return;
}

orc_comms_impl_t *orc_tcpprovider_create(char *hostname, int port)
{
	orc_comms_impl_t *impl = (orc_comms_impl_t*) calloc(sizeof(orc_comms_impl_t), 1);
	
	tcpprovider_t *t = (tcpprovider_t*) calloc(sizeof(tcpprovider_t), 1);
	t->hostname = strdup(hostname);
	t->port = port;
	
	impl->connect = tcp_connect;
	impl->disconnect = tcp_disconnect;
	impl->_p = t;

	return impl;
}


