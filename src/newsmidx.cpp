#include "def.h"

#include "newsthread.h"
#include "newslartn.h"
#include "newslsubj.h"
#include "newslauth.h"
#include "newsldate.h"

#include "newslsum.h"
#include "newssauth.h"
#include "newssdate.h"

#include "newsmidx.h"

NewsListIndex *
MakeIndex (const char * grp, const char * file, ostream * pstm,
	int sortType, int *pnDir, int min_artnum, BOOL bBinCol)
{
	*pnDir = 1;
	switch(sortType)
	{
	case SORT_R_ARTNUM:
	case SORT_X_ARTNUM:
		*pnDir = -1;
	case SORT_ARTNUM:
		return new NewsListArtNum (grp, file, pstm, min_artnum);

	case SORT_R_DATE:
	case SORT_X_DATE:
		*pnDir = -1;
	case SORT_DATE:
		return new NewsListDate (grp, file, pstm, min_artnum);

	case SORT_R_SUM_DATE:
		*pnDir = -1;
	case SORT_SUM_DATE:
		return new NewsSumDate (grp, file, pstm, min_artnum, bBinCol);

	case SORT_SUMMARY:
		return new NewsListSummary (grp, file, pstm, min_artnum, bBinCol);

	case SORT_SUBJECT:
		return new NewsListSubj (grp, file, pstm, min_artnum, bBinCol);

	case SORT_AUTHOR:
		return new NewsListAuthor (grp, file, pstm, min_artnum);

	case SORT_SUM_AUTHOR:
		return new NewsSumAuthor (grp, file, pstm, min_artnum, bBinCol);

	case SORT_THREAD:
		return new NewsThread (grp, file, pstm, min_artnum);
	}

	return NULL;
}

int
GetArtNum (const char * grp, ostream * pstm, int nArtNum,
	BOOL bNext, int sorttype, int min, BOOL bBinCol)
{
	ZString dirGroup = grp;
	for(unsigned int i = 0; i < dirGroup.length(); i++)
	{ if(dirGroup[i] == '.') dirGroup[i] = '/'; }

	int nDir;
	NewsListIndex * pindex = MakeIndex(grp, dirGroup, pstm,
		sorttype, &nDir, min, bBinCol); 
	if(pindex == NULL)
		return -1;
	if(pindex->Fetch() <= 0)
		return -1;

	int artIdx = pindex->Find(nArtNum);
	if(artIdx == -1)
		return -1;

	return pindex->GetArtNum(artIdx + (bNext?1:-1));
}
