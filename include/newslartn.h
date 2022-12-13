#ifndef __NEWSLARTN_H__
#define __NEWSLARTN_H__

#include "newslsort.h"

class NewsListArtNum : public NewsListSort
{
public:
	NewsListArtNum (const char * grp, const char * file,
		ostream * poutStat, int min = 0)
		: NewsListSort(grp, file, poutStat, min )
	{
	}

protected:
	const char * GetFileName();
	int GetOverField();
	compar GetCompar();
};
#endif // __NEWSLARTN_H__
