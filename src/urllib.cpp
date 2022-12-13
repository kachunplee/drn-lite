#include <string.h>
#include "def.h"

ostream& operator << (ostream& stm, const URLText& txt)
{
	char * p = (char *)txt.m_stg;
	char c;
	int n;
	for(;;)
	{
		n = strcspn(p, "%+");
		c = p[n];
		if(c) p[n] = 0;
		stm << p;
		switch(c)
		{
		case '%':
			stm << "%25";
			break;
		case '+':
			stm << "%2B";
		}
		if(c == 0)
			break;
		p[n] = c;
		p += n+1;
	}
	return stm;
}

const char * URLText::GetText ()
{
	if(m_pText)
		delete m_pText;

	int nLimit = strlen(m_stg);
	m_pText = new char[nLimit*3+1];
	if(m_pText == NULL)
		return NULL;

	char * p = (char *)m_stg;
	char * q = m_pText;
	for(int n = 0; n < nLimit && *p; n++, p++)
	{
		switch(*p)
		{
		case ' ':
			*q++ = '%';
			*q++ = '2';
			*q++ = '0';
			break;
		case '%':
			*q++ = '%';
			*q++ = '2';
			*q++ = '5';
			break;
		case '+':
			*q++ = '%';
			*q++ = '2';
			*q++ = 'B';
			break;
		case '=':
			*q++ = '%';
			*q++ = '3';
			*q++ = 'D';
			break;
		default:
			*q++ = *p;
		}
	}
	*q = 0;
	return m_pText;
}

ostream& operator << (ostream& stm, const URLPostText& txt)
{
	char * p = (char *) txt.m_stg;
	char c;
	int n;
	for(;;)
	{
		n = strcspn(p, "%+@!");
		c = p[n];
		if(c) p[n] = 0;
		stm << p;
		switch(c)
		{
		case '%':
			stm << "%25";
			break;
		case '+':
			stm << "%2B";
			break;
		case '@':
		case '!':
			stm << "_";
			break;
		}
		if(c == 0)
			break;
		p[n] = c;
		p += n+1;
	}
	return stm;
}
