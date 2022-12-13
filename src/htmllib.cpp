#include <string.h>
#include "def.h"

ostream& operator << (ostream& stm, const HTMLText& txt)
{
	char * p = (char *)txt.m_stg;
	if(*p == 0)
		return stm;
	char c;
	int n;
	for(;;)
	{
		n = strcspn(p, "&<");
		c = p[n];
		if(c) p[n] = 0;
		stm << p;
		switch(c)
		{
		case '&':
			stm << "&amp;";
			break;
		case '<':
			stm << "&lt;";
		}
		if(c == 0)
			break;
		p[n] = c;
		p += n+1;
	}
	return stm;
}

char * HTMLText::GetText ()
{
	if(m_pText)
		delete m_pText;

	int nLimit = strlen(m_stg);
	if(nLimit == 0)
		return NULL;

	m_pText = new char[nLimit*5+1];
	if(m_pText == NULL)
		return NULL;

	char * p = (char *)m_stg;
	char * q = m_pText;
	for(int n = 0; n < nLimit && *p; n++, p++)
	{
		switch(*p)
		{
		case '&':
			*q++ = '&';
			*q++ = 'a';
			*q++ = 'm';
			*q++ = 'p';
			*q++ = ';';
			break;
		case '<':
			*q++ = '&';
			*q++ = 'l';
			*q++ = 't';
			*q++ = ';';
			break;
		default:
			*q++ = *p;
		}
	}
	*q = 0;
	return m_pText;
}
