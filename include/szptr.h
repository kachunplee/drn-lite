#ifndef __SZPTR_H__
#define __SZPTR_H__

#include <string.h>
#include <string>

class szptr
{
public:
	friend bool operator< (const szptr &a, const szptr &b);

	szptr (const char * p) : m_sz(p) {}
	szptr (string & p) : m_sz(p.c_str()) {}

	operator const char * () const { return m_sz; }

protected:
	const char * m_sz;
};

inline bool operator< (const szptr &a, const szptr &b)
{
	return (strcmp(a.m_sz, b.m_sz) < 0);
}

#endif // __SZPTR_H__
