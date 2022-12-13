#include "def.h"
#include "newslsum.h"
#include "newssorts.h"

static int
CmpStructArtNum (const void * p1, const void * p2)
{
	return ((SortSumStruct *)p1)->artno - ((SortSumStruct *)p2)->artno;
}

//
// This sort does not support Find
//
int
NewsSortSum::Find (ARTNUM)
{
	return -1;
}

//
// If multi-part, display article list instead of article
//
ostream &
NewsSortSum::OutSubject(ostream & stm, int n,
	const char * grp, const char * artno, const char * subj,
	int pagelen, int iFile)
{
	return OutSubjectSum(stm, n, grp, artno, subj, pagelen, iFile);
}

ostream &
NewsSortSum::OutURL(ostream & stm, int n,
	const char * grp, const char * artno, int pagelen)
{
	return OutURLSum(stm, n, grp, artno, pagelen);
}


//
int
NewsSortSum::GetMulti (int n)
{
	return GetMultiSum(n);
}

BOOL
NewsSortSum::GetArtNumList ()
{
	NewsListSummary Index(m_pGroup, m_dirGroup, m_status.GetSTM(),
		m_nMinArtNum, m_bBinCol);
	m_nArtNum = Index.Fetch();
	if(m_nArtNum <= 0)
		return FALSE;

	if(Index.Set(0, m_nArtNum) != m_nArtNum)
		return FALSE;

	m_SortSums = new SortSumStruct[m_nArtNum];
	for(int i = 0; i < m_nArtNum; i++)
	{
		m_SortSums[i].artno = Index.GetArtNum(i);
		m_SortSums[i].bytes = Index.GetBytes(i);
		m_SortSums[i].idx = Index.GetArtIdx(i);
		//m_SortSums[i].off = Index.GetArtOff(i);
	}

	qsort(m_SortSums, m_nArtNum, sizeof(m_SortSums[0]), CmpStructArtNum);

	m_ArtNumIdx = new ArtNumIdx[m_nArtNum];
	for(int i = 0; i < m_nArtNum; i++)
	{
		m_ArtNumIdx[i].artnum = m_SortSums[i].artno;
		//m_ArtNumIdx[i].off = m_SortSums[i].off;
	}

	m_status("Sorting...");
	m_status.PushSTM();

	return TRUE;
}

BOOL
NewsSortSum::EndOver ()
{
	int j = 0;
	for(int i = 0; i < m_nArtNum; i++)
	{
		if(m_pSort[i])
		{
			m_pSort[j] = m_pSort[i];
			ASSERT(m_pSort[j]->thg.artnum == m_SortSums[i].artno);
			m_pSort[j]->thg.idx = IDX_MASK | (m_SortSums[i].idx&~IDX_MASK);
			m_pSort[j]->thg.bytes = m_SortSums[i].bytes;
			++j;
		}
	}

	ASSERT(j == m_nIndex);
	delete m_SortSums;
	m_SortSums = NULL;
	return TRUE;
}
