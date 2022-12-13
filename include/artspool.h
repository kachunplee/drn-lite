#ifndef __ARTSPOOL_H__
#define __ARTSPOOL_H__

#include <list.h>

class NNTPArticle;
class Zifstream;
class ArticleSpool
{
public:
	ArticleSpool(const char * group, const char * artnum, BOOL hdronly = FALSE);
	ArticleSpool(const char * fname);
	~ArticleSpool();
	char * GetLine();
	BOOL good();
	int Getc();

	void NoFile();
	const char * File();

	const char * Errmsg ()				{ return m_errmsg; }

protected:
	NNTPArticle * m_pNNTP;
	Zifstream * m_pFile;
	ZString		m_StreamName;
	ZString		m_FileName;
	BOOL		m_bSave;
	BOOL		m_bEOF;
	ZString		m_errmsg;
		
	list<string>	m_list;
	char *		m_pLine;
};

#endif // __ARTSPOOL_H__
