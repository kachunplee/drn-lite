#ifndef __INLIB_H__
#define __INLIB_H__

#include <time.h>

class INDateTime
{
protected:
	tm		m_tm;
	
public:
	INDateTime();
	INDateTime(char * p);

	tm& time ()						{ return m_tm; }
};

class INMailName
{
protected:
	const char * m_stg;
public:
	INMailName (const char * p)			{ m_stg = p; }

	ZString & EMailAddress(ZString &);
	ZString & RealName(ZString &);
};

#endif
