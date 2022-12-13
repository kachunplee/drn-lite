#include "def.h"

#include "newsldate.h"

const char * NewsListDate::GetFileName () { return "/date.i"; }

int NewsListDate::GetOverField ()
{
	return 1<<FIELD_DATE;
}

static int CmpDate (const void * p1, const void * p2)
{
	return (*(SortStruct**)p1)->date - (*(SortStruct**)p2)->date;
}

compar NewsListDate::GetCompar ()
{
	return CmpDate;
}

BOOL NewsListDate::SetOverFld (int n, int fld, char * p)
{
	if(fld < FIELD_DATE)
		return TRUE;

	if(fld == FIELD_DATE)
	{
		INDateTime dt(p);
		m_pSort[n]->date = mktime(&dt.time());
	}

	return FALSE;			// say no more
}
