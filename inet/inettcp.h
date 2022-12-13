#ifndef __INETTCP_H__
#define __INETTCP_H__

#include <netinet/in.h>
#include "csocket.h"

class InetTCP : public CSocket
{
public:
	InetTCP (u_short port, u_long addr)
		: CSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
	{
		if(Bind(port, addr) < 0) throw(errno);
	}

	InetTCP (int s = -1)
		: CSocket(s)
	{}

	int Bind (u_short port, u_long addr = INADDR_ANY)
	{
		SockAddrIN sockaddr(port, addr);
		return CSocket::Bind(sockaddr);
	}

	int Recv (char * buf, int len, int flags = 0)
	{ return CSocket::Recv(buf, len, flags); }
	int Send (const char * buf, int len = 0, int flags = 0)
	{ return CSocket::Send(buf, len?len:strlen(buf), flags); }

	int SendRecv(const char * sbuf, int slen, char * rbuf, int rlen = 0);
};

#endif // __INETTCP_H__
