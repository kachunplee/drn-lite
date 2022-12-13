#ifndef __NEWSMIDX_H__
#define __NEWSMIDX_H__

#include "newslidx.h"

extern NewsListIndex * MakeIndex(const char * grp, const char * file,
	ostream * pstm, int sortType, int * pnDir,
	int min_artnum = 0, BOOL bBinCol = FALSE);

extern int GetArtNum(const char * grp, ostream * pstm, int nArtNum,
	BOOL bNext, int sorttype = SORT_THREAD, int min = 0, BOOL bBinCol = FALSE);

#endif // __NEWSMIDX_H__
