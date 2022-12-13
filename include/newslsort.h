#ifndef __NEWSLSORT_H__
#define __NEWSLSORT_H__

#include <time.h>
#include "newslover.h"

class SortStruct
{
public:
	struct IndexThg	thg;
	ZString			stg;
	union {
		time_t			date;
		const char *	p;
		void *			pv;
	};

public:
	SortStruct(streamoff off, ARTNUM artnum)
	{
		thg.off = off;
		thg.artnum = artnum;
		thg.bytes = 0;
		pv = NULL;
	}
};

typedef int (*compar) (const void *, const void *);

class NewsListSort : public NewsListOver
{
public:
	NewsListSort (const char * grp, const char * file,
		ostream * poutStat, int min = 0)
		: NewsListOver(grp, file, poutStat, min )
	{
		m_pSort = NULL;
	}

	~NewsListSort();

protected:
	BOOL BegOver();
	BOOL SetOver(int, ssize_t, char *);
	BOOL EndOver();

	virtual BOOL SetOverFld (int, int fld, char *);

	virtual BOOL Sort();
	virtual BOOL MakeThgs ();
	virtual void DeletePSort();

	#
	virtual int GetOverField() = 0;
	virtual compar GetCompar() = 0;

protected:
	SortStruct **		m_pSort;
};

#endif // __NEWSLSORT_H__
