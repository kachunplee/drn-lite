#ifndef __NEWSSORTS_H__
#define __NEWSSORTS_H__

#include "newslsort.h"

typedef struct
{
	ARTNUM		artno;
	unsigned	idx;
	unsigned	bytes;
}
SortSumStruct;

class NewsSortSum : public NewsListSort
{
public:
	NewsSortSum (const char * grp, const char * file,
		ostream * poutStat, int min, BOOL bBinCol)
		: NewsListSort(grp, file, poutStat, min )
	{
		m_SortSums = NULL;
		m_bBinCol = bBinCol;
	}

	~NewsSortSum ()			{ delete m_SortSums; }

	ostream & OutURL(ostream &, int, const char * grp,
		const char * artno, int pagelen);
	ostream & OutSubject(ostream &, int, const char * grp,
		const char * artno, const char * subj, int pagelen, int iFile);
	int GetMulti(int);

	int Find(ARTNUM);

protected:
	SortSumStruct * m_SortSums;
	
	BOOL GetArtNumList();
	BOOL EndOver();

protected:
	BOOL	m_bBinCol;
};

#endif // __NEWSSORTS_H__
