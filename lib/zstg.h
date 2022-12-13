#ifndef __ZSTG_H__
#define __ZSTG_H__

#include <string>
#include <strstream.h>

#include "zregex.h"

class StringStreamBuf : public strstreambuf
{
public:
	//
	// Get String Pointer
	//
	// Put pointer is reset to beginning
	// So use this pointer before next input
	//	and, of course, before object is destroyed
	//
	char * GetStg(int * = NULL);
	void Terminate(int *);
	void ResetPut ()				{ freeze(0); setp(base(), epptr()); }
};

//----------------------------------------------------------------------------//

class ZString : public string
{
public:
	ZString()												{}
	ZString(const string& x) : string(x)					{}
	ZString(const char* t) : string(t?t:"")					{}

	ZString(const string& x, size_t pos, size_t len = npos)
		: string(x, pos, len)	{}
	ZString(const char* t, size_t len)
		: string(t, len)	{}
	ZString(char c, size_t rep = 1)
		: string(rep, c)	{}

	#
	const char * chars () const							{ return c_str(); }
	operator const char * () const						{ return c_str(); }

	#
	char lastchar() const						{ return (*this)[length()-1]; }

	#
	void downcase();

	#
	void del(size_t pos, size_t len = 1);
	void DelLast(size_t len = 1);

	#
	size_t spn(const char *);
	size_t cspn(const char *);

	int index(char c, size_t n = 0)
	{ n = find(c, n); return (n == npos) ? -1 : (int) n; }

	int index(const Regex &, size_t = 0);
	int indexr(const Regex &, size_t = 0);

	#
	int gsub(const char *, const char *);
	int gsub(char, char);

	#
	bool containsSet (const char * p)	{ return (cspn(p) < length()); }

	bool contains (const char p, size_t n = 0)	 { return (find(p,0) != npos); }
	bool contains (const char * p, size_t n = 0) { return (find(p,0) != npos); }

	bool matches(const Regex &, size_t = 0);

	#
	ZString at(char c, size_t = 0);
	ZString after(char c, size_t = 0);
	ZString through(char c, size_t = 0);
	ZString before(char c, size_t = 0);

	ZString at(const Regex &, size_t = 0);
	ZString before(const Regex &, size_t = 0);
	ZString through(const Regex &, size_t = 0);
	ZString after(const Regex &, size_t = 0);

	#
	friend int readline(istream& s, ZString& x, char terminator = '\n');
};

//----------------------------------------------------------------------------//
//
// Global Functions
//
void strstrip(char * q, char c);
int hex(char c);

#endif // __ZSTG_H__
