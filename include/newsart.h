#ifndef __NEWSART_H__
#define __NEWSART_H__

#include "dgauth.h"
#include "newsmime.h"

class Zifstream;
class ArticleSpool;
class NewsArticle
{
protected:
	ZString&		m_stgArticle;
	NewsOption&		m_opt;
	NewsMIME		m_NewsMIME;
	ZString			m_stgSubject;
	int			m_nHeaderCount;
	int			m_nArticleNo;

	MD5_CTX *		m_pctxRoot;
#ifdef DIGESTPWD
	MD5_CTX			m_ctxCmd;
	char			m_Digest[DIGBUFLEN];
#endif

public:
	NewsArticle (ZString& stg, NewsOption & opt, int nArtNo)
		: m_stgArticle(stg), m_opt(opt), m_nArticleNo(nArtNo)
	{
		m_pctxRoot = NULL;
	}

	~NewsArticle ()			{ if(m_pctxRoot) delete m_pctxRoot; }

	friend ostream& operator <<(ostream&, NewsArticle&);

	void OutToolbar(ostream&, const char *);
	void OutEnd(ostream&, Zifstream&);
	void pr_from(ostream&, ZString &);
	void pr_newsgroups(ostream&, ZString &);
	void pr_references(ostream&, ZString &);
	void pr_contdisp(ostream&, ZString&);
	void pr_filename(ostream&);
	void pr_RFC_body(ostream&, ArticleSpool &);
	void pr_MIME_headers(ostream&);
	void pr_MIME_body(ostream&);
	void pr_strline(ostream& stm, ZString&, BOOL);
	void pr_line(ostream&, char *);
	int pr_link(ostream&, char *);

	void init_mime();
	void chk_mime_header(char *);
};

#endif //__NEWSART_H__
