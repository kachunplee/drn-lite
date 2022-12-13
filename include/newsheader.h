#ifndef __NEWSHEADER_H__
#define __NEWSHEADER_H__

class Zifstream;
class NewsHeader
{
protected:
	ZString&		m_stgArticle;
	NewsOption&		m_opt;
	ZString			m_stgSubject;
	int				m_nArticleNo;

public:
	NewsHeader (ZString& stg, NewsOption & opt)
		: m_stgArticle(stg), m_opt(opt)
	{
	}

	friend ostream& operator <<(ostream&, NewsHeader&);

	void OutToolbar(ostream&, const char *, const char *);
	void OutEnd(ostream&, ZString &, Zifstream&);
	ZString pr_from(ostream&, char*);
	ZString pr_newsgroups(ostream&, char*);
};

#endif //__NEWSHEADER_H__
