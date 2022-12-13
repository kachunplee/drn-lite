#ifndef __NEWSLAUTH_H__
#define __NEWSLAUTH_H__

#include "newslsort.h"

class NewsListAuthor : public NewsListSort
{
public:
	NewsListAuthor (const char * grp, const char * file,
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
#endif // __NEWSLAUTH_H__
