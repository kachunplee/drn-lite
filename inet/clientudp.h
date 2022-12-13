#ifndef __CLIENTUDP_H__
#define __CLIENTUDP_H__

#include <time.h>
#include <string>

#include "inetudp.h"

const int tRetryServer = 600;	// sec


class desc_server;
class ClientUDP 
{
public:
	ClientUDP(const char *, u_short port = 0, int buflen = 2048);
	~ClientUDP();
	int Getfd () const	{ return m_udp ? m_udp->Getfd() : -1; }

	int SetCmd (char * fmt, ...);
	char * SendRecv (char *, int *);

protected:
	char *		m_pBuf;
	int			m_nBuf;
	int			m_iBuf;

	InetUDP *	m_udp;
	desc_server *	m_servers;
	int		m_nServer;
	int		m_cServer;
	unsigned	m_seqnum;
	time_t		m_tConnect;

protected:
	bool connect();
};

#endif // __CLIENTUDP_H__
