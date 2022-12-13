#include <stdio.h>
#include <string.h>
#include "def.h"

#include "userinfo.h"
#include "newslsum.h"

const char * NewsListSummary::GetFileName ()
{
	return m_bBinCol ? "/summary.b" : "/summary.s" ;
}

//
// This sort does not support Find
//
int
NewsListSummary::Find (ARTNUM)
{
	return -1;
}

//
// If multi-part, display article list instead of article
//
ostream &
NewsListSummary::OutSubject(ostream & stm, int n,
	const char * grp, const char * artno, const char * subj,
	int pagelen, int iFile)
{
	return OutSubjectSum(stm, n, grp, artno, subj, pagelen, iFile);
}

ostream &
NewsListSummary::OutURL(ostream & stm, int n,
	const char * grp, const char * artno, int pagelen)
{
	return OutURLSum(stm, n, grp, artno, pagelen);
}

//
int
NewsListSummary::GetMulti (int n)
{
	return GetMultiSum(n);
}

//
BOOL
NewsListSummary::MakeThgs ()
{
	ZString *pSubj = NULL;
	ZString *pAuthor = NULL;;
	int begmulti = -1;
	int bRe = 0;
	int nextpart = 1;
	
	int j = 0;
	int n = 1;
	int k = 0;
	unsigned TotalSize = 0;
	unsigned CompletedSize = 0;

	for(int i = 0; i < m_nIndex; i++)
	{
		SortExt * pext = (SortExt*)m_pSort[i]->pv;
		if(pext == NULL)
			return FALSE;

		if( ((pext->nPart&~BINARY_ARC) > 0 && (k=(pext->nNum&~RE_MASK)) > 0)
				|| (m_bBinCol && ((pext->nPart&BINARY_ARC) != 0)))
		{
			//
			// This is a binary archive or multi-part articles
			//
			if(begmulti >= 0
				// check if subjects are the same
				&& *pSubj == m_pSort[i]->stg
				&& *pAuthor == pext->author
				&& (bRe == (pext->nNum&RE_MASK)))
			{
				n++;
				TotalSize += m_pSort[i]->thg.bytes;
				if(((pext->nPart&BINARY_ARC) == 0)
					&& (k == nextpart))	
				{
					CompletedSize += m_pSort[i]->thg.bytes;
					nextpart++;
				}
				continue;
			}

			//
			// subject different... finish up last multi
			//
			SetIdx(begmulti, n, nextpart, TotalSize, CompletedSize);

			//
			// Start new one
			//
			bRe = (pext->nNum&RE_MASK);

			n = 1;
			nextpart = 1;
			TotalSize = m_pSort[i]->thg.bytes;
			if(((pext->nPart&BINARY_ARC) == 0)
				&& (k == nextpart))
			{
				CompletedSize = TotalSize;
				nextpart++;
			}

			pSubj = &m_pSort[i]->stg;
			pAuthor = &pext->author;
			begmulti = j;		// news multi at idx j (i will be moved to j)
		}
		else
		{
			// Not a multi-part
			// finish up last multi if any
			SetIdx(begmulti, n, nextpart, TotalSize, CompletedSize);
			begmulti = -1;		// not a multi
		}

		if(i != j)
		{
			delete m_pSort[j];
			m_pSort[j] = m_pSort[i];
			m_pSort[i] = NULL;
		}
		m_pSort[j]->thg.idx = IDX_MASK;
		j++;
	}

	// finish up the last multi
	SetIdx(begmulti, n, nextpart, TotalSize, CompletedSize);
	m_nIndex = j;

	return NewsListSort::MakeThgs();
}

void
NewsListSummary::SetIdx (int i, int n, int nextpart,
	unsigned TotalSize, unsigned CompletedSize)
{
	if(i < 0)
		return;

	int nPart = ((SortExt*)m_pSort[i]->pv)->nPart;
	if(nPart&BINARY_ARC)
		m_pSort[i]->thg.bytes = TotalSize;
	else
	{
		if(nextpart > (nPart&~BINARY_ARC))
			m_pSort[i]->thg.bytes = CompletedSize | PART_MULTI | PART_COMPLETE;
		else
			m_pSort[i]->thg.bytes = TotalSize | PART_MULTI;
	}
			
	m_pSort[i]->thg.idx = (n<<IDX_BIT) | IDX_MASK;
}
