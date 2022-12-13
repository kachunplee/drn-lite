#ifndef __NEWSPOST_H__
#define __NEWSPOST_H__

class Zifstream;
class NewsPost
{
protected:
	ZString&		m_stgArticle;
	NewsOption&		m_opt;
	ZString			m_stgSubject;
	int				m_nArticleNo;
	BOOL			m_bAttach;

public:
	NewsPost (ZString& stg, NewsOption & opt, BOOL bAttach)
		: m_stgArticle(stg), m_opt(opt)
	{
		m_bAttach = bAttach;
	}

	friend ostream& operator <<(ostream&, NewsPost&);

	void OutToolbar(ostream&, const char *, const char *);
	void OutEnd(ostream&, ZString &, Zifstream&);
};

#endif //__NEWSPOST_H__
