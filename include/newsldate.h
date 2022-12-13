#ifndef __NEWSLDATE_H__
#define __NEWSLDATE_H__

#include "newslsort.h"

class NewsListDate : public NewsListSort
{
public:
	NewsListDate (const char * grp, const char * file,
		ostream * poutStat, int min = 0)
		: NewsListSort(grp, file, poutStat, min )
	{
	}

protected:
	const char * GetFileName();
	int GetOverField();
	compar GetCompar();

	BOOL SetOverFld (int, int, char *);
};
#endif // __NEWSLDATE_H__
