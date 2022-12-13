#ifndef __CSOCKET_H__
#define __CSOCKET_H__

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <strings.h>

#include <fcntl.h>

class SockAddr
{
public:
	virtual struct sockaddr * operator() () = 0;
	virtual int SizeOf () const = 0;
};

class SockAddrIN : public SockAddr
{
public:
	SockAddrIN (u_short port = 0, u_long addr = INADDR_ANY)
	{
		m_sockaddr.sin_family = AF_INET;
		m_sockaddr.sin_port = htons(port);
		m_sockaddr.sin_addr.s_addr = htonl(addr);
	}

	SockAddrIN (char * pa, u_short port = 0)
	{
		m_sockaddr.sin_family = AF_INET;
		m_sockaddr.sin_port = htons(port);
		bcopy(pa, (char *) &m_sockaddr.sin_addr.s_addr,
			sizeof(m_sockaddr.sin_addr.s_addr));
	}

	struct sockaddr * operator() ()
				{ return (struct sockaddr *) &m_sockaddr; }
	struct sockaddr_in * GetAddr ()
				{ return &m_sockaddr; }
	int SizeOf () const	{ return sizeof(m_sockaddr); }

protected:
	sockaddr_in	m_sockaddr;
};

class CSocket
{
public:
	CSocket (u_short family, int type, int proto = 0)
	{
		m_socket = socket(family, type, proto);
		if(m_socket < 0) throw(errno);
	}

	CSocket (int s = -1)
	{ m_socket = s; }

	~CSocket ()
	{ if(m_socket >0 && close(m_socket) < 0) throw(errno); }

	void Setfd (int s)		{ m_socket = s; }
	int Getfd () const		{ return m_socket; }

	int Bind (SockAddr & myAddr)
	{ return bind(m_socket, myAddr(), myAddr.SizeOf()); }

	int Connect (SockAddr & servAddr)
	{ return connect(m_socket, servAddr(), servAddr.SizeOf()); }

	int Listen (int backlog = 5)
	{ return listen(m_socket, backlog); }

	// Anyone really use l?
	int Accept (SockAddr & peerAddr)
	{ socklen_t l = peerAddr.SizeOf();
	  return accept(m_socket, peerAddr(), &l); }

	int SetNDELAY ()
	{ return fcntl(m_socket, F_SETFL, O_NDELAY); }

	int Recv (char * buf, int len, int flags = 0)
	{ return recv(m_socket, buf, len, flags); }

	int Send (const char * buf, int len, int flags = 0)
	{ return send(m_socket, (char *)buf, len, flags); }

	int Read (char * buf, int len)
	{ return read(m_socket, buf, len); }

	int Write (const char * buf, int len)
	{ return write(m_socket, (char *)buf, len); }

protected:
	int 	m_socket;
};

#endif // __CSOCKET_H__
