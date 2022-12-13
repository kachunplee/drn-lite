#ifndef __URLLIB_H__
#define __URLLIB_H__

class URLText
{
protected:
	const char *	m_stg;
	char * m_pText;
	
public:
	URLText (const char * p)		{ m_stg = p; m_pText = NULL; }
	~URLText ()				{ delete m_pText; }

	friend ostream& operator <<(ostream&, const URLText&);
	const char * GetText();
};

class URLPostText
{
protected:
	const char *	m_stg;
	
public:
	URLPostText (const char * p)		{ m_stg = p; }

	friend ostream& operator <<(ostream&, const URLPostText&);
};

#endif // __URLLIB_H__
