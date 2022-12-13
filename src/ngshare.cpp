#include <sys/types.h>
#include <stdio.h>

#include "def.h"
#include "nntpact.h"

#include "userinfo.h"

#include "ngshare.h"


NewsGroupShare::NewsGroupShare ()
{
	m_pActive = NULL;
}

NewsGroupShare::~NewsGroupShare ()
{
	delete m_pActive;
}

BOOL NewsGroupShare::BeginList ()
{
	try
	{
		m_pActive = new NNTPActive;
	}
	catch(const char * errmsg)
	{
		m_errmsg = errmsg;
		return FALSE;
	}
	catch(int err)
	{
		m_errmsg = strerror(err);
		return FALSE;
	}

	return TRUE;
}

char * NewsGroupShare::GetLine (ZString & stgName)
{
	char * p = NULL;
	try
	{
		p = m_pActive->GetLine();
	}
	catch(const char * errmsg)
	{
		m_errmsg = errmsg;
		return NULL;
	}
	catch(int err)
	{
		m_errmsg = strerror(err);
		return NULL;
	}

	if(p == NULL)
		return NULL;

	//
	// Look for end of group name
	//
	char * q;
	for(q = p; *q && !isspace(*q); ++q) ;
	if(*q == 0)
		return NULL;

	*q = 0;
	stgName = p;
	return q+1;
}

int
NewsGroupShare::GetGroupIndex (const char * pGroup)
{
	if(!Open())
		return -1;
	return FindGroup(pGroup);
}

//
// Helpers
//
BOOL
NewsGroupShare::Open ()
{
#if 0
	if(m_pActive == NULL)
		m_pActive = new NewsActive;

	if(m_pActive.Open() == FALSE)
			return FALSE;

	return m_IdxActive.Index(m_Active);
#endif
	return FALSE;
}

int NewsGroupShare::FindGroup (const char * pGroup)
{
#if 0
	unsigned int nLen = strlen(pGroup);
	if(nLen == 0)
		return -1;

	char * p;
	int i, nResult;
	int n = m_pshm->m_nSize-1;
	int l = 0;
	for(;;)
	{
		i = (n+l)/2;

		p = m_pshm->m_Actives + m_pshm->m_Entries[i].m_offsetActive;
		nResult = strncasecmp(pGroup, p, nLen);
		if(nResult == 0)
		{
			if(nLen == strlen(p))
				return i;
			nResult = -1;
		}

		if(nResult > 0)
 		{
			if(i == n)
				break;
			l = i+1;
		}
		else
		{
			if(i == l)
				break;
			n = i-1;
		}
//		if(i == nResult)
//		{
			i = nResult > 0 ? l : n;
			p = m_pshm->m_Actives + m_pshm->m_Entries[i].m_offsetActive;
			if(strncasecmp(pGroup, p, nLen) == 0)
			{
				if(nLen == strlen(p))
					return i;
			}
//			break;
//		}
//		i = nResult;
	}
#endif
	return -1;
}
