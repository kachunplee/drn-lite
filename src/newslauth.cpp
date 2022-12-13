#include <string.h>

#include "def.h"
#include "newslauth.h"

const char * NewsListAuthor::GetFileName () { return "/author.i"; }

int NewsListAuthor::GetOverField ()
{
	return 1<<FIELD_FROM;
}

static int CmpStg (const void * p1, const void * p2)
{
	return strcasecmp((*(SortStruct**)p1)->stg.chars(),
				(*(SortStruct**)p2)->stg.chars());
}

compar NewsListAuthor::GetCompar ()
{
	return CmpStg;
}

BOOL NewsListAuthor::SetOverFld (int n, int fld, char * p)
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
