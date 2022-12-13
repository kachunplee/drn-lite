#ifndef __CLIENTTCP_H__
#define __CLIENTTCP_H__

#include <string>

#include "inettcp.h"

class desc_server;
class ClientTCP 
{
public:
	ClientTCP(const char *, u_short port = 0, int buflen = 4096);
	~ClientTCP();

	int Getfd () const	{ return m_tcp ? m_tcp->Getfd() : -1; }

	int GetCServer () const { return m_tcp ? m_cServer : -1; }

	int SetCmd(char * fmt, ...);
	char * SendCmd(char * = NULL, char * = NULL, int * = NULL);
	char * GetLine(int * = NULL);

	int Send(const char * buf, int len = 0);

protected:
	char *	m_pBuf;
	int		m_nBuf;

	int		m_iBuf;

	int		m_nOut;
	int		m_iOut;

	InetTCP *	m_tcp;
	desc_server *	m_servers;
	int		m_nServer;
	int		m_cServer;
	time_t		m_tConnect;

protected:
	bool connect();

};

#endif // __CLIENTTCP_H__
