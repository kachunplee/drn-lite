#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "def.h"

//
static char * weekdays [] =
{
	"Sunday",
	"Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturaday",
};

static char * months [] =
{
	"Janurary", "Feburary", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "Decemeber",
};

//
INDateTime::INDateTime ()
{
	time_t t;
	::time(&t);
	m_tm = *gmtime(&t);
}

INDateTime::INDateTime (char * p)
{
#ifndef SOLARIS
	m_tm.tm_zone = "GMT";
	m_tm.tm_gmtoff = 0;
#endif
	m_tm.tm_isdst = 0;
	m_tm.tm_wday = -1;
	m_tm.tm_hour = 0;
	m_tm.tm_min = 0;
	m_tm.tm_sec = 0;

	int i,n;
	//
	// Get day of week
	//
	char *q;
	for(q = p; *q && !isdigit(*q); q++)
	{
		if(*q == ' ' || *q == ',')
		{
			n = q-p;
			for(i = 0; i < 7; i++)
				if(strncasecmp(p, weekdays[i], n) == 0)
				{
					m_tm.tm_wday = i;
					break;
				}
			q++;
			break;
		}
	}
	for(p = q; *p && !isdigit(*p); p++);				// skip weekday

	//
	// Get day of month
	//
	m_tm.tm_mday = atoi(p);
	while(*p && (isdigit(*p) || *p == ' ')) p++;		// skip day

	//
	// Get month
	//
	m_tm.tm_mon = 0;
	for(q = p; *q; q++)
	{
		if(!isalpha(*q))
		{
			n = q-p;
			for(i = 0; i < 12; i++)
				if(strncasecmp(p, months[i], n) == 0)
				{
					m_tm.tm_mon = i;
					break;
				}
			break;
		}
	}
	for(p = q; *p && !isdigit(*p); p++);				// skip month

	//
	// Get Year
	//
	m_tm.tm_year = atoi(p);
	if(m_tm.tm_year < 70)
		m_tm.tm_year += 100;
	else if(m_tm.tm_year > 1900)
		m_tm.tm_year -= 1900;
	while(*p && isdigit(*p)) p++;
	while(*p && *p == ' ') p++;		// skip year

	//
	// Get Time
	//
	time_t tz, t=0;
	char tzdiff[128];
	if(*p == 0)
		return;
	i = sscanf(p, "%2d:%2d:%2d %s\t", 
		&m_tm.tm_hour, &m_tm.tm_min, &m_tm.tm_sec, tzdiff);
	if( (i == 4) && strcasecmp(tzdiff, "GMT") && 
		(strlen(tzdiff) > 3) )
	{
		if (tzdiff[0] == '-')
			t -= int(atoi(tzdiff+1)/100) * 3600 + 
				int(atoi(tzdiff+3)) * 60;
		else if (tzdiff[0] == '+')
			t += int(atoi(tzdiff+1)/100) * 3600 + 
				int(atoi(tzdiff+3)) * 60;
		else if(isdigit(tzdiff[0]))
			t += int(atoi(tzdiff)/100) * 3600 + 
				int(atoi(tzdiff+2)) * 60;
#ifndef SOLARIS
		m_tm.tm_zone = "";
		m_tm.tm_gmtoff = t;
#endif
		tz = ::mktime(&m_tm);
#ifndef SOLARIS
		tz += m_tm.tm_gmtoff - t;	///
#endif
		if (m_tm.tm_isdst)			///
			tz -= 3600;				///
		m_tm = *(::gmtime(&tz));
#ifndef SOLARIS
		m_tm.tm_zone = "GMT";
#endif
	}
}

//----------------------------------------------------------------------------//
// Mail Name Formats:
//	(Real Name) Email Name
//	Real Name <Email Name>
//
ZString & INMailName::EMailAddress(ZString & stg)
{
	if(!m_stg || m_stg == "")
		return stg = "";

	const char *p,*q;

	//
	// Real Name <Email Mail>
	//
	p = strchr(m_stg, '<');
	if(p && (q=strchr(++p, '>')))
	{
		stg = ZString(p, q-p);
		return stg;
	}

	//
	// (Real Name) Email Name
	//
	if(m_stg[0] == '(' && (p = strchr(m_stg, ')')))
	{
		p++;
		q = m_stg+strlen(m_stg)-1;
	}
	//
	// Email Name (Real Name)
	//
	else if((q = strchr(m_stg, '(')) && strchr(q, ')'))
	{
		q--;
		p = m_stg;
	}
	else
	{
		//
		// Last resort - take the whole string
		p = m_stg;
		q = m_stg+strlen(m_stg)-1;
	}

	for(; q>m_stg && *q==' '; q--);				// strip trailing space
	for(p = m_stg; p<q && *p==' '; p++);		// strip leading space
	if(*p == '"' && *q == '"') { p++; q--; }		// strip quote
	if(*p == '<' && *q == '>') { p++; q--; }		// strip <>
	stg = ZString(p, q-p+1);

	return stg;
}

ZString & INMailName::RealName(ZString & stg)
{
	if(!m_stg || m_stg == "")
		return stg = "";

	const char *p,*q;

	//
	// (Real Name) ...
	//
	p = strchr(m_stg, '(');
	if(p && (q=strchr(++p, ')')))
	{
		if(*p == '"' && q[-1] == '"') { p++; q--; }
		stg = ZString(p, q-p);
		return stg;
	}

	//
	// Real Name <Email Mail>
	//
	p = strrchr(m_stg, '<');
	if(p && p > m_stg && strchr(p, '>'))
	{
		q = p-1;
	}
	else
	{
		q = m_stg+strlen(m_stg)-1;
	}

	for(; q>m_stg && *q==' '; q--);				// strip space
	for(p = m_stg; p<q && *p==' '; p++);		// skip space
	if(p != q && *p == '"' && *q == '"') { p++; q--; }		// strip quote
	if(*p == '<' && *q == '>') { p++; q--; }		// strip <>
	stg = ZString(p, q-p+1);

	return stg;
}
