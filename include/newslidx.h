#ifndef __NEWSLIDX_H__
#define __NEWSLIDX_H__

#include "status.h"

class NewsListIndex
{
public:
	NewsListIndex(const char * grp, const char * file,
		ostream * poutStat, int min = 0);
	virtual ~NewsListIndex();

	virtual int Fetch() = 0;
	virtual int Find (ARTNUM) = 0;
	virtual ARTNUM GetArtNum (int) = 0;
	virtual unsigned GetSize (int) = 0;

	virtual int Set(int nBeg, int nGet, int nDir) = 0;
	virtual BOOL Get(int idx, ZString &) = 0;

	virtual ostream & OutURL(ostream &, int, const char * grp,
		const char * artno, int pagelen);
	virtual ostream & OutSubject(ostream &, int, const char * grp,
		const char * artno, const char * subj, int pagelen, int iFile);

	virtual void Bad () = 0; 

	virtual int GetMulti(int);

protected:
	const char * m_pGroup;
	const char * m_dirGroup;
	Status		m_status;

	int			m_nMinArtNum;
	int			m_nArticle;
};

//
// GetMulti returns:
//	    0 -> No info about multi part
//
const int NLINE_MULTI  	  = 0x40000000;
const int NLINE_COMPLETED = 0x20000000;
const int NLINE_BINARC 	  = 0x10000000;
const int NLINE_NPART 	  = 0x0fffffff;

#endif // __NEWSLIDX_H__
