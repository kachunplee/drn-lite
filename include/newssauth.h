#ifndef __NEWSSAUTH_H__
#define __NEWSSAUTH_H__

#include "newssorts.h"

class NewsSumAuthor : public NewsSortSum
{
public:
	NewsSumAuthor (const char * grp, const char * file,
		ostream * poutStat, int min, BOOL bBinCol)
		: NewsSortSum(grp, file, poutStat, min, bBinCol)
	{
	}

protected:
	const char * GetFileName();
	int GetOverField();
	compar GetCompar();

	BOOL SetOverFld (int, int, char *);
};
#endif // __NEWSSAUTH_H__
