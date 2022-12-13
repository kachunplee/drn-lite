#include "def.h"
#include "newslib.h"

static const char *
MultiPartNum (const char* p, const char** ppNum, const char** ppTotal)
{
	while( *p && !isdigit(*p) ) p++;
	*ppNum = p;

	while( *p && isdigit(*p) ) p++;
	while( *p && !isdigit(*p) ) p++;
	*ppTotal = p;

	while( *p && isdigit(*p) ) p++;
	return p;
}

static const Regex expMultP1("[[(][0-9]+(/|[ ]+of[ ]+)[0-9]+([])]|$)");

BOOL
NewsSubject::IsMultiPart (ZString & stg, int * pnTotal, int * pnNum)
{
	const char * pNum;
	int nTotal;
	BOOL ret = IsMultiPart(pnNum, &nTotal, &pNum, NULL, 0, 1);
	if(ret == FALSE || nTotal <= 1)
		return FALSE;

	if(pnTotal) *pnTotal = nTotal;
	stg = m_stg.substr(0, pNum - m_stg.chars());
	while(isdigit(*pNum)) pNum++;
	stg += pNum;
	return TRUE;
}

BOOL
NewsSubject::IsMultiPart (int *pnNum, int* pnTotal,
			const char** ppNum, const char **ppTotal, int min, int minTotal)
{
	const char *pNum, *pTotal;
	int nNum, nTotal;

	const char *ptNum, *ptTotal;
	int ntNum, ntTotal;

	BOOL bR = FALSE;
	const char *p = m_stg.chars();
	const char *q = p;
	for(int i;;)
	{
		i = m_stg.index(expMultP1, q-p);
		if(i < 0)
			break;

		q = MultiPartNum(p+i, &ptNum, &ptTotal);
		ntNum = atoi(ptNum);
		ntTotal = atoi(ptTotal);
		if(ntNum >= min && ntTotal >= minTotal && ntNum <= ntTotal)
		{
			bR = TRUE;
			nNum = ntNum;
			nTotal = ntTotal;
			pNum = ptNum;
			pTotal = ptTotal;
		}
	}

	if(!bR)
		return FALSE;

	if(pnNum)
		*pnNum = nNum;
	if(pnTotal)
		*pnTotal = nTotal;
	if(ppNum)
		*ppNum = pNum;
	if(ppTotal)
		*ppTotal = pTotal;
	return TRUE;
}

static Regex expBinType ("\\.[eE][xX][eE]"
						"|\\.[zZ][iI][pP]"
						"|\\.[rR][aA][rR]"
						"|\\.[rR][0-9][0-9]"
						"|\\.[uU][uU][eE]");

static Regex expPhotoType ("\\.[gG][iI][fF]"
						"|\\.[gG][pP][fF]"
						"|\\.[jJ][pP][eE]?[gG]"
						"|\\.[bB][mM][pP]");

static Regex expVideoType ("\\.[aA][vV][iI]"
						"|\\.[aA][sS][fF]"
						"|\\.[mM][oO][vV]"
						"|\\.[mM][pP][eE]?[gG]");

static Regex expSoundType("\\.[wW][aA][vV]"
						"|\\.[mM][iI][dD][iI]?"
						"|\\.[sS][nN][dD]"
						"|\\.[sS][wW][aA]"
						"|\\.[aA][iI][fF][cCfF]?"
						"|\\.[aA][fF][cC]"
						"|\\.[mM][pP][23uU]"
						"|\\.[mM]1[aA]"
						"|\\.[rR][aA][mM]?"
						"|\\.[aA][uU]");

int
NewsSubject::GetFileExt (int * pidx)
{
	int i;
	if(pidx == NULL)
		pidx = &i;

	if((*pidx = stg().index(expPhotoType)) >= 0)
		return EXT_PHOTO;

	if((*pidx = stg().index(expVideoType)) >= 0)
		return EXT_VIDEO;

	if((*pidx = stg().index(expBinType)) >= 0)
		return EXT_BINARIES;

	if((*pidx = stg().index(expSoundType)) >= 0)
		return EXT_SOUND;

	return EXT_UNKNOWN;
}
