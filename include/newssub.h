#ifndef __NEWSSUB_H__
#define __NEWSSUB_H__

#include <string>
#include <map.h>

typedef map<string, int> GrpSubMap;

class NewsGroupSubscribe
{
public:
	NewsGroupSubscribe(const char *);

	GrpSubMap & GetMap ()				{ return m_map; }

	void Set(const char *, int = 0);
	bool Find(const char *);

	void DumpMap ();

protected:
	GrpSubMap	m_map;
};

#endif // -_NEWSSUB_H__
