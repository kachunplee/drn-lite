#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "dmsg.h"
#include "cnntp.h"

CNNTP::CNNTP (const char * servers, const char * auths)
	: ClientTCP(servers, 119)
{
		// Get Header
		char * p = SendCmd();
		DMSG(1, "%s", p);

		if(strncmp(p, "200 ", 4) != 0) throw(p);
		if(strstr(p, "posting") == NULL)
		{
			char * p = SendCmd("mode reader\r\n");
			DMSG(1, "%s", p);
			if(strncmp(p, "200 ", 4) != 0) throw(p);
		}

		int n = GetCServer();
		if(n >= 0 && auths)
		{
			string userid;
			string passwd;
			for(int i = 0; (i < n) && *auths; auths++)
			{
				if(*auths == ';') i++;
			}

			if(*auths)
			{
				const char * q = auths;
				while(*auths && *auths != '/' && *auths != ';') auths++;
				userid = string(q, auths-q);
				if(*auths == '/')
				{
					q = ++auths;
					while(*auths && *auths != ';') auths++;
					passwd = string(q, auths-q);
				}
			}

			if(userid.length())
			{
				SetCmd("authinfo user %s\r\n", userid.c_str());
				p = SendCmd();
				if(strncmp(p, "381 ", 4) != 0) throw(p);

				SetCmd("authinfo pass %s\r\n", passwd.c_str());
				p = SendCmd();
				if(strncmp(p, "281 ", 4) != 0) throw(p);
			}
		}
}

CNNTP::~CNNTP ()
{
		char * p = SendCmd("QUIT\r\n");
		DMSG(1, "%s", p);
}

char * CNNTP::GetLine ()
{
	int n;
	char * p = GetLine(&n);
	if(p[n-1] == '\r') p[--n] = 0;
	if(p[0] == '.')
	{
		switch(p[1])
		{
		case '.':
			return p+1;

		case 0:
			return NULL;
		}
	}

	return p;
}
