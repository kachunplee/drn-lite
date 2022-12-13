#ifndef __NEWSGRP_H__
#define __NEWSGRP_H__

#include "status.h"

class NewsGroupShare;
class NewsGroupList
{
protected:
	ZString&	m_stgGroupList;
	NewsOption&	m_opt;

public:
	NewsGroupList (ZString& stg, NewsOption& opt)
		: m_stgGroupList(stg), m_opt(opt)
	{}

	friend ostream& operator <<(ostream&, NewsGroupList&);
};

class ListNewsGroup
{
protected:
	NewsGroupList *		m_grp;
	NewsGroupShare *	m_pGroupShare;
	Status				m_status;

	int					m_nGrp;

public:
	ListNewsGroup(NewsGroupList *);
	virtual ~ListNewsGroup();

	int GetNumArt(const char *);
	BOOL IsModerate(const char *);
	virtual BOOL open() = 0;
	virtual const char * GetLine(Regex &, int *, BOOL *) = 0;

protected:
	char * GetGroup(ZString &);
};

class ListNewsGroupL : public ListNewsGroup
{
protected:
	Zifstream *	m_pstmIn;
	ZString		m_stg;

public:
	ListNewsGroupL (NewsGroupList * grp) : ListNewsGroup(grp)
	{ m_pstmIn = NULL; }

	virtual ~ListNewsGroupL ()								{ delete m_pstmIn; }

	virtual BOOL open();
	virtual const char * GetLine(Regex &, int *, BOOL *);
};

class ListNewsGroupG : public ListNewsGroup
{
protected:
	Zifstream *	m_pstmIn;
	SBOOL		m_bTrailDot;
	SBOOL		m_bSav;

	ZString *	m_pstgIn;
	ZString *	m_pstgGroup;
	ZString *	m_pstg1Group;
	ZString *	m_pstgSav;

	int			m_nGroup;

public:
	ListNewsGroupG (NewsGroupList * grp, BOOL b = FALSE)
		: ListNewsGroup(grp), m_bTrailDot(b)
	{ m_pstgIn = NULL; m_pstgSav = NULL; m_pstgGroup = NULL;
		m_pstg1Group = NULL;
		m_nGroup = 0; m_bSav = FALSE;}
	~ListNewsGroupG ()
	{ delete m_pstgIn; delete m_pstgSav; delete m_pstgGroup;
		delete m_pstg1Group; }

	virtual BOOL open();
	virtual const char * GetLine(Regex &, int *, BOOL *);
};

#endif //__NEWSGRP_H__
