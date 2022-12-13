#include <stdio.h>

#include "def.h"
#include "newslidx.h"

//
NewsListIndex::NewsListIndex (const char * grp,
		const char * dirGroup, ostream * poutStat, int min)
	: m_status(poutStat)
{
	//
	// Group specification
	//
	m_pGroup = grp;
	m_dirGroup = dirGroup;
	m_nMinArtNum = min;

	//
	m_nArticle = 0;
}

NewsListIndex::~NewsListIndex()
{
}

ostream &
NewsListIndex::OutSubject (ostream & stm, int n,
	const char * grp, const char * artno, const char * subj,
	int pagelen, int)
{
	OutURL(stm, n, grp, artno, pagelen);
	stm	<< HTMLText(subj) << "</a>";
	return stm;
}

ostream &
NewsListIndex::OutURL (ostream & stm, int, const char * grp,
		const char * artno, int)
{
	stm << "<a href=\"" << NEWSBIN << "/wwwnews?"
		<< URLText(grp) << "/" << artno << "\">";
	return stm;
}

int
NewsListIndex::GetMulti (int)
{
	return 0;
}
