#ifndef __NEWSLIST_H__
#define __NEWSLIST_H__

#include <stdio.h>
#include "dgauth.h"

struct ThreadOffStruct
{
	ARTNUM		nArtnum;
	streamoff	stmoff;
	int			srchIdx;
	int			iDepth;
};

class NewsListIndex;
class NewsArticleList
{
protected:
	ZString&	m_stgGroup;
	NewsOption& m_opt;
	NewsOption	m_optCmd;

	ZString		m_dirGroup;;

	int			m_idxSortType;
	int			m_nDir;
	NewsListIndex * m_pIndex;

	int			m_nArticle;
	ARTNUM		m_nMinArtNum;

	MD5_CTX *		m_pctxRoot;
#ifdef DIGESTPWD
	MD5_CTX			m_ctxCmd;
	char			m_Digest[DIGBUFLEN];
#endif

public:
	NewsArticleList(ZString& stg, NewsOption& opt);
	~NewsArticleList();

protected:
	friend ostream& operator <<(ostream&, NewsArticleList&);

	int GetIndex(ostream*, int);

	ARTNUM Goto(ARTNUM, int = 0);

	int OutArticles(ostream&, ARTNUM = 0);
	void OutArticle(ostream&, const char *, int idx, int pagelen,
			BOOL bBold = FALSE);
	void OutBegin(ostream&, char *);
	void OutLink(ostream&);
	void OutEnd(ostream&);
	void OutPageLinks(ostream&);

	int GetPageLen();
};

#endif //__NEWSLIST_H__
