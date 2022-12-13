#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "def.h"
#include "szptr.h"
#include "status.h"

#include "newslsort.h"

#ifndef FREEBSD
extern "C" void
mergesort(void *base, size_t nmemb, size_t size,
	int (*cmp)(const void *, const void *));
#endif

NewsListSort::~NewsListSort ()
{
	DeletePSort();
}

//
// Read headers from overview file
//
BOOL
NewsListSort::BegOver ()
{
	m_pSort = new SortStruct* [m_nArtNum];
	if(m_pSort == NULL)
		return FALSE;

	for(int i = 0; i < m_nArtNum; i++ )
		m_pSort[i] = NULL;

	return TRUE;
}

BOOL
NewsListSort::SetOver (int n, ssize_t off, char * over)
{
	if(m_pSort[n] == NULL)
	{
		m_pSort[n] = new SortStruct(off, m_ArtNumIdx[n].artnum);
		m_nIndex++;
	}

	if(GetOverField() == 0)
		return TRUE;

	int nField = 1;
	char *q = over;
	BOOL more = TRUE;
	for(char *p = over; *p; p++)
	{
		switch(*p)
		{
		case '\t':
		case '\r':
		case '\n':
			*p = 0;
			if(more)
				more = SetOverFld(n, nField, q);
			if(nField++ == FIELD_BYTES)
			{
				m_pSort[n]->thg.bytes = strtoul(q, NULL, 10);
				if(!more) break;
			}
			q = p+1;
		}
	}

	return TRUE;
}

BOOL
NewsListSort::SetOverFld (int, int, char *)
{
	return FALSE;
}

BOOL
NewsListSort::EndOver ()
{
	//
	// compact array
	//
	int j = 0;
	for(int i = 0; i < m_nArtNum; i++)
	{
		if(m_pSort[i])
		{
			m_pSort[j] = m_pSort[i];
			m_pSort[j]->thg.idx = j;
			++j;
		}
	}

	ASSERT(j == m_nIndex);
	return TRUE;
} 

BOOL
NewsListSort::Sort ()
{
	compar fun = GetCompar();
	if(fun)
	{
		m_status("Sorting articles...");
		mergesort(m_pSort, m_nIndex, sizeof(m_pSort[0]), fun);
	}
	
	int ret = MakeThgs();

	m_status.SetDefault();
	return ret;
}

BOOL
NewsListSort::MakeThgs ()
{
	int i;
	unsigned n;
	m_Thgs = new IndexThg [m_nIndex];
	for(i = 0; i < m_nIndex; i++)
		m_Thgs[i] = m_pSort[i]->thg;

	if((m_Thgs[0].idx&IDX_MASK) != IDX_MASK)
	  for(i = 0; i < m_nIndex; i++)
	  {
		n = m_pSort[i]->thg.idx&IDX_MASK;
		m_Thgs[n].idx = i | (m_Thgs[n].idx & ~IDX_MASK);
	  }

	return TRUE;
}

void
NewsListSort::DeletePSort ()
{
	if(m_pSort)
	{
		for(int i = 0; i < m_nArtNum; i++ )
			delete m_pSort[i];

		delete [] m_pSort;
		m_pSort = NULL;
	}
}
