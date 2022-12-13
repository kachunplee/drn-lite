#ifndef __NEWSSDATE_H__
#define __NEWSSDATE_H__

#include "newssorts.h"

class NewsSumDate : public NewsSortSum
{
public:
	NewsSumDate (const char * grp, const char * file,
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
#endif // __NEWSSDATE_H__
