#ifndef __NEWSLSUBJ_H__
#define __NEWSLSUBJ_H__

#include "newslsort.h"

typedef struct
{
	unsigned short nPart;
	unsigned short nNum;
	ZString filenum;
	ZString author;
} SortExt;

const int RE_MASK	= 0x8000;			// high bit of nPart
const int BINARY_ARC = 0x8000;			// high bit of nNum

class NewsListSubj : public NewsListSort
{
public:
	NewsListSubj (const char * grp, const char * file,
		ostream * poutStat, int min, BOOL bBinCol)
		: NewsListSort(grp, file, poutStat, min )
	{
		m_bBinCol = bBinCol;
		m_pExt = NULL;
	}
	~NewsListSubj ()
	{
		delete m_pExt;
	}

protected:
	const char * GetFileName();
	int GetOverField();
	compar GetCompar();

	BOOL SetOverFld (int, int, char *);
	void DeletePSort();

protected:
	SortExt *	m_pExt;
	BOOL	m_bBinCol;
};
#endif // __NEWSLSUBJ_H__
