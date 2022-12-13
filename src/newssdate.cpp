#include "def.h"

#include "newssdate.h"

const char * NewsSumDate::GetFileName ()
{ return m_bBinCol ? "/date.b" : "/date.s"; }

int NewsSumDate::GetOverField ()
{
	return 1<<FIELD_DATE;
}

static int CmpDate (const void * p1, const void * p2)
{
	return (*(SortStruct**)p1)->date - (*(SortStruct**)p2)->date;
}

compar NewsSumDate::GetCompar ()
{
	return CmpDate;
}

BOOL NewsSumDate::SetOverFld (int n, int fld, char * p)
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
