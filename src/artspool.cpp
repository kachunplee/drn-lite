#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "def.h"

#include "nntpart.h"
#include "artspool.h"

ArticleSpool::ArticleSpool (const char * group, const char * artnum, BOOL hdronly)
{
	m_bSave = TRUE;
	m_bEOF = FALSE;
	m_pNNTP = NULL;
	m_pFile = NULL;
	m_pLine = NULL;

	try
	{
		m_pNNTP = new NNTPArticle(group, artnum, hdronly);
	}
	catch(const char * errmsg)
	{
		m_errmsg = errmsg+4;
		DMSG(0, "ArticleSpool: %s", errmsg);
	}
	catch(int err)
	{
		m_errmsg = strerror(err);
		DMSG(0, "ArticleSpool: %s", m_errmsg.chars());
	}
}

ArticleSpool::ArticleSpool (const char * fname)
{
	m_bSave = FALSE;
	m_bEOF = FALSE;
	m_pNNTP = NULL;
	m_pFile = NULL;
	m_pLine = NULL;

	m_StreamName = fname;
	m_pFile = new Zifstream(fname, ios::nocreate|ios::in);
}

ArticleSpool::~ArticleSpool ()
{
	if(m_pFile)
	{
		m_pFile->close();
		delete m_pFile;
	}
	delete m_pNNTP;
	if(m_FileName.length() > 0)
		unlink(m_FileName);
}

BOOL
ArticleSpool::good ()
{
	if (m_pFile)
		return m_pFile->good();

	return m_pNNTP != NULL;
}

char *
ArticleSpool::GetLine ()
{
	if(m_pFile)
	{
		if(m_bEOF)
			return NULL;
		char * p = m_pFile->GetLine();
		if(!m_pFile->NextLine())
			m_bEOF = TRUE;
		return p;
	}

	if(m_pNNTP == NULL)
		return NULL;

	char * p = NULL;
	if(m_pLine)
	{
		p = m_pLine;
		m_pLine = NULL;
		return p;
	}

	try
	{
		p = m_pNNTP->GetLine();
		if(m_bSave)
			m_list.push_back((const char *)p);
		return p;
	}
	catch(const char * errmsg)
	{
		DMSG(0, "ArticleSpool::GetLine %s", errmsg);
	}
	catch(int err)
	{
		DMSG(0, "ArticleSpool::GetLine %s", strerror(err));
	}

	return NULL;
}

int
ArticleSpool::Getc ()
{
	if(m_pFile)
		return m_pFile->get();

#if 0
	if(m_ungetc >= 0)
	{
		int c = m_ungetc;
		m_ungetc = -1;
		return c;
	}
#endif

	if(m_pLine == NULL)
	{
		m_pLine = GetLine();
		if(m_pLine == NULL)
			return EOF;
	}

	if(*m_pLine == 0)
	{
		m_pLine = NULL;
		return '\n';
	}

	return *m_pLine++;
}

void
ArticleSpool::NoFile ()
{
	m_bSave = FALSE;
	m_list.erase(m_list.begin(), m_list.end());
}

const char *
ArticleSpool::File ()
{
	if(m_pFile)
		return m_FileName.chars();
	
	if(m_FileName.length() == 0)
	{
		char szFile [strlen(TMPDIR) + 10];
		sprintf(szFile, "%s/as.XXXXXX", TMPDIR); 
		m_FileName = mktemp(szFile);
		ofstream tmpf(m_FileName);

		for(list<string>::iterator iter = m_list.begin();
			iter != m_list.end(); iter++)
		{
			tmpf << *iter << '\n';
		}

		NoFile();

		char * p;
		while((p = GetLine()))
			tmpf << p << '\n';
		delete m_pNNTP;
		m_pNNTP = NULL;
	}

	return m_FileName.chars();
}
