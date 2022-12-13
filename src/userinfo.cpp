#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>

#ifndef LINUX
#include <ndbm.h>
#endif

#include "def.h"
#include "userinfo.h"

BOOL IsTokenIn (const char * p, const char * token)
{
	const char * q;
	int l = strlen(token);
	while((q = strstr(p, token)))
	{
		// got string
		if((p==q || q[-1] == ' ') && (q[l] == 0 || q[l] == ' '))
			return TRUE;

		p = q+1;
	}

	return FALSE;
}

BOOL UserInfo::Init (ostream& stm, const char * pUser, const char * pHost,
	const char * pAddr)
{
	ZString accesslist;

	m_UserName = pUser;
	m_HostName = pHost;
	m_HostAddr = pAddr;

	m_HostDomain.empty();

	SetUserType(0);

#ifndef USERINFO
	return TRUE;
#else
	if(m_UserName.length() == 0)
	{
		// Must be in by Host Access
		if(GetHostInfo(&accesslist))
		{
			if(accesslist[0] == '*')
				SetUserType((accesslist.length() > 1)?1:2);
		}
		return TRUE;
	}

	//
	// Got User Name - make sure the name is not using Host Access 
	//
	ZString userinfo;
	if(!GetUserInfo(userinfo))
		return FALSE;			// can't find user? Impossible

	if(IsTokenIn(userinfo, "dlimit"))
	{
		stm << "Content-Type: text/html" << endl << endl
			<< DEF_BODY_TAG << endl;
		stm << "<h3>You have reached the download limit. Please check your email or check our <a href=http:" 
			<< HOME_SERVER
			<< "/policy.htm#usage>policy</a> for details.</h3>" << endl;
		// bad...
		exit(1);
	}

	if(!IsTokenIn(userinfo, "host"))
	{
		//
		// Regular User
		//
		if(!IsTokenIn(userinfo, "-"))
			SetUserType(2);
		return TRUE;
	}

	//
	// That is a host - check UserName must match Host Addr
	//
	if(GetHostInfo(&accesslist))
	{
		if(accesslist[0] == '*')
			SetUserType((accesslist.length()>1)?1:2);
		return TRUE;
	}

	//
	// No - user login from Wrong Domain
	//
	return FALSE;
#endif
}

BOOL UserInfo::GetUserInfo(ZString & userinfo)
{
#ifndef USERINFO
	userinfo = "drn";
#else
	DBM * pDB = dbm_open(USERINFODB, O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if(pDB == NULL)
		return FALSE;

	datum user, info;
	user.dptr = (char *)m_UserName.chars();
	user.dsize = m_UserName.length();
	info = dbm_fetch(pDB, user);
	if(info.dsize == 0)
	{
		dbm_close(pDB);
		return FALSE;
	}

	userinfo = ZString(info.dptr, info.dsize);
	dbm_close(pDB);
#endif
	return TRUE;
}

BOOL UserInfo::GetHostInfo(ZString * pAccessList /*= NULL*/)
{
#ifdef USERINFO
	ZString stg;
	char *p, *q;
	ifstream lstFile(HOSTINFODB, ios::nocreate|ios::in);
	while(lstFile.good())
	{
		if(readline(lstFile, stg) <= 0)
			break;

		for(p = ((char *)stg.chars()); *p && isspace(*p); p++);	// skip blanks

		for(q = p; *q; q++)
		{
			if(*q == '#' || *q == '\r' || *q == '\n')
			{
				*q = 0;
				break;
			}
		}

		if(*p == 0)									// empty line
			continue;

		char *fld[5];
		int i = 0;
		fld[i] = p;
		for(q = p; *q; q++)
		{
			if(*q == ':')
			{
				*q = 0;
				fld[++i] = q+1;
			}
		}

		if (i != 4)
			continue; /* Malformed line. */

		if(m_UserName != fld[2])
			continue;

		if (!(m_HostAddr.length()>0 && wildmat(m_HostAddr.chars(), fld[0])) &&
			!(m_HostName.length()>0 && wildmat(m_HostName.chars(), fld[0])))
			continue;

		if(pAccessList)
		{
			for(i = strlen(fld[4])-1; i>=0 && isspace(fld[4][i]); i--)
				fld[4][i] = 0;
			*pAccessList = fld[4];
		}

		// get port limit
		p = fld[1];
		while(*p && !isdigit(*p))
			p++;
		m_nPortLimit = atoi(p);

		while(*p && isdigit(*p))
			p++;
		m_HostDomain = p;
		return TRUE;
	}

#endif
	return FALSE;
}
