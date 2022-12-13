#ifndef __NEWSLSUM_H__
#define __NEWSLSUM_H__

#include "newslsubj.h"

class NewsListSummary : public NewsListSubj
{
public:
	NewsListSummary(const char * grp, const char * file,
		ostream * poutStat, int min, BOOL bBinCol)
		: NewsListSubj(grp, file, poutStat, min, bBinCol)
	{
	}

	int Find(ARTNUM);
	ostream & OutSubject(ostream &, int, const char * grp,
		const char * artno, const char * subj, int pagelen, int iFile);
	ostream & OutURL(ostream &, int, const char * grp,
		const char * artno, int pagelen);

	int GetMulti(int);

protected:
	const char * GetFileName();
	BOOL MakeThgs();
	void SetIdx(int, int, int, unsigned, unsigned);
};
#endif // __NEWSLSUM_H__
