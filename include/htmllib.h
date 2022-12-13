#ifndef __HTMLLIB_H__
#define __HTMLLIB_H__

class HTMLText
{
protected:
	const char *	m_stg;
	char * 			m_pText;
	
public:
	HTMLText (const char * p)					{ m_stg = p; m_pText = NULL; }
	~HTMLText ()								{ delete m_pText; }

	friend ostream& operator <<(ostream&, const HTMLText&);
	char * GetText();
};

#endif // __HTMLLIB_H__
