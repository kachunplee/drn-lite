#include <string.h>
#include "zstg.h"

//----------------------------------------------------------------------------//
//
// Strings globals
//
int hex
(char c)
{
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if(c >= 'a' && c <= 'f')
		return c - 'A' + 10;
	return 0;
}

void strstrip
(char * q, char c)
{
	char * p  = q;
	do
	{
		if(*p == c)
		{
			++p;
		}

		if(p != q)
			*q = *p;
		q++;
	} while(*p++);
}


//----------------------------------------------------------------------------//
//
// StringStreamBuf
//
char *
StringStreamBuf::GetStg (int * pLen)
{
	char * p = str();
	Terminate(pLen);
	ResetPut();
	return p;
}

void
StringStreamBuf::Terminate (int * pLen)
{
	if(pLen != NULL)
	{
		*pLen = pptr() ? pptr() - pbase() : 0;
	}
	else
		if(pptr()) *pptr() = 0;					// Terminate string
}

//----------------------------------------------------------------------------//
//
// ZString
//
void
ZString::del (size_t pos, size_t len)
{
	if(len > 0)
		erase(pos, len);
}

void
ZString::DelLast (size_t len)
{
	size_t l = length();
	erase((l<len) ? 0 : (l-len), len);
}

void
ZString::downcase ()
{
	string::iterator stgend = end();
	for(string::iterator iter = begin(); iter != stgend; ++iter)
	{
		int c = *iter;
		if(isupper(c)) *iter = c - 'A' + 'a';
	}
}

//
// Finds
//
size_t
ZString::spn (const char * p)
{
	return strspn(c_str(), p);
}

size_t
ZString::cspn (const char * p)
{
	return strcspn(c_str(), p);
}

int
ZString::index (const Regex & ex, size_t n)
{
	if(length() < n) return -1;

	regmatch_t match;
	int result = ex.RegExec(chars()+n, 1, &match);
	if(result) return -1;
	return match.rm_so+n;
}

int
ZString::indexr (const Regex & ex, size_t n)
{
	if(length() < n) return -1;

	regmatch_t match;
	int result = ex.RegExec(chars()+n, 1, &match);
	if(result) return -1;
	return match.rm_eo+n;
}

bool
ZString::matches (const Regex & ex, size_t n)
{
	return (index(ex, n) == 0);
}

//
// Replaces
//
int
ZString::gsub (const char * old, const char * that)
{
	int l = strlen(old);
	int k = strlen(that);
	int i = 0;

	for(unsigned n = 0; (n = find(old, n)) != npos; i++, n += k)
	{
		replace(n, l, that);
	}

	return i;
}

int
ZString::gsub (char old, char that)
{
	int i = 0;
	for(unsigned n = 0; (n = find(old, n)) != npos; i++, n++)
	{
		(*this)[n] = that;
	}
	
	return i;
}

//
// Substrs
//
ZString
ZString::at (char c, size_t n)
{
	n = find(c, n);
	return (n==npos) ? ZString() : substr(n, 1);
}

ZString
ZString::before (char c, size_t n)
{
	size_t l = find(c, n);
	return (l==npos) ? ZString() : substr(n,l-n);
}

ZString
ZString::through (char c, size_t n)
{
	size_t l = find(c, n);
	return (l==npos) ? ZString() : substr(n,l-n+1);
}

ZString
ZString::after (char c, size_t n)
{
	n = find(c, n);
	return (n==npos) ? ZString() : substr(n+1);
}

ZString
ZString::at (const Regex & ex, size_t n)
{
	if(length() < n) return ZString();

	regmatch_t match;
	int result = ex.RegExec(chars()+n, 1, &match);

	if(result) return ZString();

	return substr(match.rm_so+n, match.rm_eo-match.rm_so);
}

ZString
ZString::before (const Regex & ex, size_t n)
{
	int l = index(ex, n);
	return (l < 0) ? ZString() : substr(n, l-n);
}

ZString
ZString::through (const Regex & ex, size_t n)
{
	int l = indexr(ex, n);
	return (l < 0) ? ZString() : substr(n, l-n);
}

ZString
ZString::after (const Regex & ex, size_t n)
{
	int l = indexr(ex, n);
	return (l < 0) ? ZString() : substr(l);
}

//
// Beware - gnu String::readline strips white spaces
//	but this does not.
//
int readline(istream& s, ZString& x, char term)
{
	do
	{
		getline(s, x, term);
		if(s.eof()) return 0;
		if(s.fail()) return -1;
	}
	while(x.length() == 0);

	return x.length();
}
