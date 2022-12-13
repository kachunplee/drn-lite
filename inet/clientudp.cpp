#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <iostream.h>
#include <string.h>

#include "dmsg.h"

#include "cinet.h"

#include "inetudp.h"
#include "clientudp.h"

class desc_server
{
public:
	string name;
	u_short port;
};

ClientUDP::ClientUDP (const char * servers, u_short port, int buflen)
	: m_nBuf(buflen), m_iBuf(sizeof(m_seqnum)),
		 m_udp(NULL), m_cServer(0)
{
	const char * p;
	int l,n;
	m_nServer = 1;
	for(p = servers; *p; p++)
		if(*p == ';') ++m_nServer;

	m_servers = new desc_server [m_nServer];

	for(n = 0, p = servers; *p; ++n)
	{
		l = strcspn(p, ":;");
		m_servers[n].name = string(p,l);
		m_servers[n].port = port;
		p += l;
		l = 0;
		if(*p == ':') {
			m_servers[n].port = atoi(++p);
			p += strcspn(p, ";");
		}

		DMSG(1, "%s:%d",
			m_servers[n].name.c_str(), m_servers[n].port);

		if(*p == ';') ++p;
	}

	m_pBuf = new char [buflen+4];
	m_seqnum = (unsigned) getpid() << 16;
}

ClientUDP::~ClientUDP ()
{
	delete m_udp;
	delete [] m_servers;
}

bool
ClientUDP::connect ()
{
	m_udp = new InetUDP(0, INADDR_ANY);

	u_long addr;
	for(; m_cServer < m_nServer; m_cServer++)
	{
		addr = AtoInetAddr(m_servers[m_cServer].name.c_str());
		if(addr == INADDR_NONE)
		{
			DMSG(0, "Can't resolve %s",
				m_servers[m_cServer].name.c_str());
			continue;
		}

		SockAddrIN saddr((char *)&addr, m_servers[m_cServer].port);
		if(m_udp->Connect(saddr) >= 0)
		{
			// got one
			m_tConnect = time(NULL);
			return true;
		}

		DMSG(0, "Can't connect to %s:%d",
			m_servers[m_cServer].name.c_str(),
			m_servers[m_cServer].port);
	}

	// No servers is avaliable
	DMSG(0, "No server avaliable");
	delete m_udp;
	m_udp = NULL;
	return false;
}

int
ClientUDP::SetCmd (char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	m_iBuf += vsnprintf (m_pBuf+m_iBuf, m_nBuf-m_iBuf, fmt, args);
	return m_nBuf-m_iBuf;
}

char *
ClientUDP::SendRecv (char * rbuf, int *pl)
{
	if((m_cServer > 0) && 
		((time(NULL) - m_tConnect) > tRetryServer))
	{
		m_tConnect = time(NULL);
		m_cServer = 0;
		DMSG(0, "Retrying servers");
		delete m_udp;
		m_udp = NULL;
	}

	if(m_udp == NULL)
		if(!connect())
			return NULL;

	int n;
	char *pBuf;
	int nBuf;
	for(pBuf = m_pBuf, nBuf = m_iBuf;;)
	{
		*(unsigned *)m_pBuf = m_seqnum;
 		n = m_udp->SendRecv(pBuf, nBuf, rbuf, *pl);
		if(n < 0)
		{
			m_seqnum++;

			// Try next server
			++m_cServer;
			DMSG(0, "Trying next server");
			delete m_udp;
			if(connect())
			{
				// set buffer to send packet to new server
				pBuf = m_pBuf;
				nBuf = m_iBuf;
				continue;
			}

			m_iBuf = sizeof(m_seqnum);
			*pl = -1;
			return NULL;
		}

		n -= sizeof(m_seqnum);
		if((n >= 0)  && (m_seqnum == * (unsigned *) rbuf))
		{
			m_seqnum++;
			m_iBuf = sizeof(m_seqnum);
			*pl = n;
			return rbuf+sizeof(m_seqnum);
		}

		DMSG(0, "Wrong sequence number %u<>%u",
			m_seqnum, *(unsigned*)rbuf);

		// no need to resend
		pBuf = NULL;
		nBuf = 0;
	}
}
