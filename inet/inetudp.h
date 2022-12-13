#ifndef __INETUDP_H__
#define __INETUDP_H__

#include <netinet/in.h>
#include "csocket.h"
#include "rtt.h"

class InetUDP : public CSocket
{
public:
	InetUDP (u_short port, u_long addr)
		: CSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP), m_pRTT(NULL)
	{
		if(Bind(port, addr) < 0) throw(errno);
	}

	InetUDP (int s = -1)
		: CSocket(s), m_pRTT(NULL)
	{}

	~InetUDP ()
	{ delete m_pRTT; }

	int Bind (u_short port, u_long addr = INADDR_ANY)
	{
		SockAddrIN sockaddr(port, addr);
		return CSocket::Bind(sockaddr);
	}

	int Recv (SockAddrIN & addr, char * buf, int len, int flags = 0)
	{ int l = addr.SizeOf();
	  return recvfrom(m_socket, buf, len, flags, addr(), &l); }

	int Send (SockAddrIN & addr, const char * buf, int len, int flags = 0)
	{ return sendto(m_socket, (char*)buf, len, flags, addr(), addr.SizeOf()); }

	int Recv (char * buf, int len, int flags = 0)
	{ return CSocket::Recv(buf, len, flags); }
	int Send (const char * buf, int len, int flags = 0)
	{ return CSocket::Send(buf, len, flags); }

	int SendRecv(const char * sbuf, int slen, char * rbuf, int rlen);

protected:
	RTT *	m_pRTT;
};

#endif // __INETUDP_H__
