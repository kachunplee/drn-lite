#ifndef __NEWSTHREAD_H__
#define __NEWSTHREAD_H__

#include "newslover.h"

class ThreadLink;
struct ThreadStruct
{
	struct IndexThg	thg;

	//
	int					out;
	ThreadLink *		link;

	ZString				msgid;
	ZString				ref;
};


class NewsThread : public NewsListOver
{
public:
	NewsThread (const char * grp, const char * file,
		ostream * poutStat, int min = 0)
		: NewsListOver(grp, file, poutStat, min )
	{
		m_pThreads = NULL;
	}

	~NewsThread();

	ostream & OutSubject(ostream &, int, const char * grp,
		const char * artno, const char * subj, int, int);

protected:
	const char * GetFileName();

	BOOL BegOver();
	BOOL SetOver(int, ssize_t, char *);
	BOOL EndOver();

	BOOL Sort();

	#
	int	PutThread(int, ThreadLink *, unsigned);
	void DeletePThreads();

protected:
	ThreadStruct **		m_pThreads;
};

#endif // __NEWSTHREAD_H__
