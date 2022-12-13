#include <string.h>

#include "def.h"
#include "newssauth.h"

const char * NewsSumAuthor::GetFileName ()
{ return m_bBinCol ? "/author.b" : "/author.s"; }

int NewsSumAuthor::GetOverField ()
{
	return 1<<FIELD_FROM;
}

static int CmpStg (const void * p1, const void * p2)
{
	return strcasecmp((*(SortStruct**)p1)->stg.chars(),
				(*(SortStruct**)p2)->stg.chars());
}

compar NewsSumAuthor::GetCompar ()
{
	return CmpStg;
}

BOOL NewsSumAuthor::SetOverFld (int n, int fld, char * p)
{
	if(fld < FIELD_FROM)
		return TRUE;

	if(fld == FIELD_FROM)
	{
		INMailName email(p);
		email.RealName(m_pSort[n]->stg);
	}

	return FALSE;			// say no more
}
