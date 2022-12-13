#ifndef __NEWSINEWS_H__
#define __NEWSINEWS_H__

class Zifstream;
class NewsINews
{
protected:
	NewsOption&		m_opt;

public:
	NewsINews (NewsOption & opt)
		: m_opt(opt)
	{
	}

	friend ostream& operator <<(ostream&, NewsINews&);

	void OutToolbar(ostream&, const char *, const char *);
	void OutEnd(ostream&, ZString &, Zifstream&);
};

#endif //__NEWSINEWS_H__
