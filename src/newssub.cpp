#include "zio.h"
#include "dmsg.h"
#include "newssub.h"

NewsGroupSubscribe::NewsGroupSubscribe (const char * pName)
{
	Zifstream of(pName, ios::nocreate|ios::in);
	if(of.good())
	{
		do
		{
			m_map[of.GetLine()] = 0;
		}
		while(of.NextLine());
	}

	if(debug) DumpMap();
}

void
NewsGroupSubscribe::Set (const char * p, int n)
{
	m_map[p] = n;
}

bool
NewsGroupSubscribe::Find (const char * p)
{
	return m_map.find(p) != m_map.end();
}

void
NewsGroupSubscribe::DumpMap ()
{
	for(GrpSubMap::iterator iter = GetMap().begin();
			iter != GetMap().end(); iter++)
	{
		DMSG(0, "%s = %d", (*iter).first.c_str(), (*iter).second);
	}
}
