#include <string.h>
#include "def.h"
#include "newslib.h"
#include "newslsubj.h"

const char * NewsListSubj::GetFileName ()
{ return m_bBinCol ? "/subject.b" : "/subject.i"; }

int NewsListSubj::GetOverField ()
{
	return (1<<FIELD_SUBJ)|(1<<FIELD_FROM);
}

static int CmpSubj (const void * p1, const void * p2)
{
	SortStruct * s1 = *(SortStruct**)p1;
	SortStruct * s2 = *(SortStruct**)p2;
	int res = strcasecmp(s1->stg.chars(), s2->stg.chars());
	if(res != 0)
		return res;

	// Part size
	int n1 = ((SortExt*)s1->pv)->nPart;
	int n2 = ((SortExt*)s2->pv)->nPart;

	res = (n2&BINARY_ARC) - (n1&BINARY_ARC);
	if(res != 0)
		return res;

	BOOL bArc = ((n1&BINARY_ARC) != 0);
	if(!bArc)
	{
		res = n1 - n2;
		if(res != 0)
			return res;
	}

	// Part number & RE
	n1 = ((SortExt*)s1->pv)->nNum;
	n2 = ((SortExt*)s2->pv)->nNum;
	res = (n1&RE_MASK) - (n2&RE_MASK);
	if(res != 0)
		return res;

	res = strcasecmp(((SortExt*)s1->pv)->author.chars(),
								((SortExt*)s2->pv)->author.chars());
	if(res != 0)
		return res;

	res = n1 - n2;
	if(res != 0)
		return res;

	return bArc ? strcasecmp(((SortExt*)s1->pv)->filenum.chars(),
					((SortExt*)s2->pv)->filenum.chars()) : 0;
}

compar NewsListSubj::GetCompar ()
{
	return CmpSubj;
}

BOOL
NewsListSubj::SetOverFld (int n, int fld, char * p)
{
	if(fld != FIELD_SUBJ && fld != FIELD_FROM)
		return (fld < FIELD_FROM);		// FIELD_FROM > FIELD_SUBJ

	int nPart, nNum;
	SortExt * pext;
	if(m_pSort[n]->pv == NULL)
	{
		pext = new SortExt;
		m_pSort[n]->pv = pext;
	}
	else
		pext = (SortExt*) m_pSort[n]->pv;

	if(fld == FIELD_FROM)
	{
		pext->author = p;
		return FALSE;
	}

	bool bRe = (strncasecmp(p, "Re:", 3) == 0);
	if(bRe)
		for(p += 3; isspace(*p); p++) ;

	NewsSubject subj(p);
	if(subj.IsMultiPart(m_pSort[n]->stg, &nPart, &nNum))
	{
		//
		// Limit to 32K parts
		//
		pext->nNum = nNum | (bRe?RE_MASK:0);
		pext->nPart = nPart;

		return TRUE;
	}

	pext->nNum = (bRe?RE_MASK:0);

	if(m_bBinCol && subj.GetFileExt(&nNum) != EXT_UNKNOWN)
	{
		char buf[subj.stg().length()+1];
		char * bp = buf;
		const char * r;

		const char * q = p+nNum-1;			// before file ext
		for(; q >= p; q--)
		{
			if(isspace(*q)) break;
		}

		for(r = p; r < q; r++)
			if(isalpha(*r) || *r < 0) *bp++ = *r;

		r = p+nNum+1;
		while(isdigit(*r) || isalpha(*r)) r++;
		q++;
		pext->filenum = subj.stg().substr(q-p, r-q);

		for(; *r; r++)
			if(isalpha(*r) || *r < 0) *bp++ = *r;

		*bp = 0;
		m_pSort[n]->stg = buf;

		DMSG(1, "%s\n%s %s", p, buf, pext->filenum.chars());

		pext->nPart = BINARY_ARC;
		return TRUE;
	}

	pext->nPart = 0;
	m_pSort[n]->stg = p;
	return TRUE;			// say no more
}

void
NewsListSubj::DeletePSort ()
{
	if(m_pSort)
	{
		for(int i = 0; i < m_nArtNum; i++ )
			delete (SortExt*)m_pSort[i]->pv;
	}
	NewsListSort::DeletePSort();
}
