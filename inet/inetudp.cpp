#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "dmsg.h"
#include "inetudp.h"

int
InetUDP::SendRecv (const char * sbuf, int slen, char * rbuf, int rlen)
{
	fd_set fdset;
	struct timeval timeout;
	int i;

	if(m_pRTT == NULL) m_pRTT = new RTT;

	for(m_pRTT->reset();;)
	{
		if(slen > 0)
		{
			i=Send(sbuf, slen);
			if(i != slen)
			{
				DMSG(0,
				  "InetUDP: couldnt send packet (%d/%d): %s",
					i, slen, strerror(errno));
				return i;
			}
		}

		timeout.tv_usec = 0;
		timeout.tv_sec = m_pRTT->start(); 

		FD_ZERO(&fdset);
		FD_SET(Getfd(), &fdset);

		if(m_pRTT->IsReXmit())
			DMSG(0, "InetUDP: ReTransmit (%ld)", timeout.tv_sec);

		i = select(Getfd()+1, &fdset, NULL, NULL, &timeout);
		if(i == 0)
		{
			if(m_pRTT->timeout() < 0)
			{
				DMSG(0, "InetUDP: Server timeout");
				return -1;
			}

			continue;
		}

		if(i < 0)
		{
			DMSG(0, "InetUDP: select error: %s",
				strerror(errno));
			return i;
		} 

		if(!FD_ISSET(Getfd(), &fdset))
		{
			DMSG(0, "InetUDP: fd not set: %s",
				strerror(errno));
			return -1;
		}

		return Recv(rbuf, rlen);
	}
}
