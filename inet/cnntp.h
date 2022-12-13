#ifndef __CNNTP_H__
#define __CNNTP_H__

#include "clienttcp.h"

class CNNTP :  public ClientTCP
{
public:
	CNNTP(const char * servers, const char * auths);
	~CNNTP();

	char * GetLine(int * p)		{ return ClientTCP::GetLine(p); }
	char * GetLine();
};

#endif // __CNNTP_H__
