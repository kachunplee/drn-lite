#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include <iostream.h>
#include <string.h>

#include "dmsg.h"

#include "cinet.h"

#include "inettcp.h"
#include "clienttcp.h"

#ifndef INADDR_NONE
#define INADDR_NONE     0xffffffff      /* -1 return */ 
#endif

class desc_server
{
public:
	string name;
	u_short port;
};

ClientTCP::ClientTCP (const char * servers, u_short port, int buflen)
	: m_nBuf(buflen), m_iBuf(0),
		m_nOut(0), m_iOut(0),
		 m_tcp(NULL), m_cServer(0)
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

	m_pBuf = new char [buflen+1];
}

ClientTCP::~ClientTCP ()
{
	delete m_tcp;
	delete [] m_servers;
}

bool
ClientTCP::connect ()
{
	if(m_tcp)
		return true;

	m_tcp = new InetTCP(0, INADDR_ANY);

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
		if(m_tcp->Connect(saddr) >= 0)
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
	delete m_tcp;
	m_tcp = NULL;
	return false;
}

int
ClientTCP::SetCmd (char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	m_iBuf += vsnprintf (m_pBuf+m_iBuf, m_nBuf-m_iBuf, fmt, args);
	return m_nBuf-m_iBuf;
}

char *
ClientTCP::SendCmd (char * buf, char * OKcode, int * pLen)
{
	if(!connect())
		return NULL;

	if(buf && buf[0])
		m_iBuf += snprintf (m_pBuf+m_iBuf, m_nBuf-m_iBuf, "%s", buf);

	DMSG(2, "ClientTCP::Send %s", m_pBuf);
	m_nOut = m_tcp->SendRecv(m_pBuf, m_iBuf, m_pBuf, m_nBuf);
	m_iBuf = 0;
	if(m_nBuf <= 0)
		return NULL;

	m_iOut = 0;
	char * p = GetLine(pLen);
	if(OKcode)
	{
		if(strncmp(p, OKcode, strlen(OKcode)) != 0)
		{
			throw(p);
			return NULL;
		}
	}

	return p;
}

char *
ClientTCP::GetLine (int * pLen)
{
	for(;;)
	{
		int nLen = m_nOut - m_iOut;
		if(nLen == 0)
			m_nOut = m_iOut = 0;
		else
		{
			char * p = NULL;
			for(p = m_pBuf+m_iOut; p < &m_pBuf[m_nOut]; p++)
				if(*p == '\n')
					break;

			// No '\n' or no more input or full buffer
			if(*p == '\n' || nLen == 0 || (m_iOut == 0 && m_nOut == m_nBuf))
			{
				*p++ = 0;
				char * q = m_pBuf+m_iOut;
				m_iOut = p-m_pBuf;
				if(pLen)
					*pLen = p-q-1;
				DMSG(2, "ClientTCP::GetLine %s", q);
				return q;
			}

			if(m_iOut > 0)
			{
				//
				// Move to front
				//
				m_nOut -= m_iOut;
				memmove(m_pBuf, m_pBuf+m_iOut, m_nOut);
				m_iOut = 0;
			}

		}

		if(!connect())
			return NULL;

		int n = m_tcp->Recv(m_pBuf+m_nOut, m_nBuf-m_nOut);
		if(n < 0)
			return NULL;

		if(n == 0 && nLen == 0)
			return NULL;

		m_nOut += n;
	}

}

int
ClientTCP::Send (const char * buf, int len = 0)
{ 
	if(!connect())
		return -1;
	return m_tcp->Send(buf, len);
}
