#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "dmsg.h"
#include "inettcp.h"

int
InetTCP::SendRecv (const char * sbuf, int slen, char * rbuf, int rlen)
{
	int i,n;
	for(i = n = 0; i < slen; i += n)
	{
		if((n=Send(sbuf+i, slen-i)) <= 0)
		{
			throw(n ? errno : (errno = EPIPE));
			return n;
		}
	}

	if(rlen == 0)
		return n;

	fd_set fdset;
	struct timeval timeout;

	timeout.tv_usec = 0;
	timeout.tv_sec = 60;

	FD_ZERO(&fdset);
	FD_SET(Getfd(), &fdset);

	i = select(Getfd()+1, &fdset, NULL, NULL, &timeout);
	if(i == 0)
	{
		DMSG(0, "InetTCP: Server timeout");
		throw(errno = ETIMEDOUT);
		return -1;
	}

	if(i < 0)
	{
		DMSG(0, "InetTCP: select error: %s",
			strerror(errno));
		throw(errno);
		return i;
	} 

	if(!FD_ISSET(Getfd(), &fdset))
	{
		DMSG(0, "InetTCP: fd not set: %s",
			strerror(errno));
		return -1;
	}

	n = Recv(rbuf, rlen);
	if(n < 0) throw(errno);
	return n;
}
