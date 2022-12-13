#include "def.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

#include "advert.h"
#include "tmplerr.h"

#ifdef GROUP_SUB
#include "newsacc.h"
#endif

#include "ngshare.h"
#include "newsgrp.h"

ostream& operator << (ostream& stm, NewsGroupList& grp)
{
#ifdef GROUP_SUB
	NewsAccess newsAccess;
	ZString stgArt = grp.m_stgGroupList;
	if(stgArt.lastchar() == '/')
		stgArt.DelLast();
	if(!newsAccess.IsGroupSubscribe(stgArt))
	{
		stm << NO_GROUP_MSG << endl;
		return stm;
	}
#endif

	TemplateError tmplerr;
	ZString tmplName = DRNTMPLDIR;
	tmplName += "/group.htm";
	Zifstream tmplFile(tmplName, ios::nocreate|ios::in);
	if(!tmplFile)
	{
		ZString err = "<h3>Template file " + tmplName;
		err += " is not found</h3>";
		tmplerr.OutError(stm, err.chars());
		return stm;
	}

	if(!tmplFile.good())
	{
		ZString err = "<h3>Template file " + tmplName;
		err += " is empty</h3>";
		tmplerr.OutError(stm, err.chars());
		return stm;
	}

	// Scan for <!--pathlink drn=group -->
	BOOL bDspGroup = FALSE;
	char * q;
	do
	{
		q = tmplFile.GetLine();

		if(strncmp(q, "<!--pathlink drn=group -->", 26) == 0)
		{
			bDspGroup = TRUE;
			break;
		}

		stm << q << endl;
	} while(tmplFile.NextLine());

	if(!bDspGroup)
	{
		stm << "<p>" << DRN_NOTICE << "</p>" << endl;

		while(tmplFile.NextLine())
		{
			q = tmplFile.GetLine();
			stm << q << endl;
		}
		return stm;
	}

	//
	// Fix up search Pattern
	//
	// Throw away trailling '|' (OR)
	//
	ListNewsGroup * lst = NULL;
	ZString grpList = "^" + grp.m_stgGroupList;
	if(grpList.lastchar() == '*')
	{
		//
		// End character = '*'
		//
		if(grpList[grpList.length()-2] == '*')
		{
			// 2 end '*' -> list as group
		    grpList.DelLast();
			grpList.DelLast();
			if(grpList.lastchar() == '.')
			{
				lst = new ListNewsGroupG(&grp, TRUE);
			}
			else
				lst = new ListNewsGroupG(&grp);
			grpList += '&';
		}
	}
	else
	{
		while(grpList.lastchar() == '|')
			grpList.DelLast();
	}

	if(lst == NULL)
		lst = new ListNewsGroupL(&grp);

	if(!lst->open())
	{
		tmplerr.OutError(stm, "<h3>Cannot open Group List file</h3>");
		delete lst;
		return stm;
	}

	grpList.downcase();
	grpList.gsub(".", "\\.");
	grpList.gsub("+", "\\+");
	grpList.gsub('?', '.');
	grpList.gsub("*", ".*");
	grpList.gsub("&", "[^.]*");

	NewsOption optCmd = grp.m_opt;

	int nEnd = grp.m_opt.GetPageLen();
	if(nEnd > 0)
	{
		nEnd += grp.m_opt.GetPageBeg();
		if(grp.m_opt.GetPageBeg() > 1)
		{
			//
			// Set up Previous Page Link
			//
			int nPrev = grp.m_opt.GetPageBeg() - grp.m_opt.GetPageLen();
			if(nPrev < 1)
				nPrev = 1;

			// Pass Options - later
			optCmd.SetPageBeg(nPrev);
			stm
				<< "<a href=" << NEWSBIN << "/wwwnews?"
				<< optCmd << URLText(grp.m_stgGroupList) << ">"
				<< "<img src=/drn/images/previous.gif alt=\"Previous\""
				<< "align=center border=0>"
				<< "</a>"
			;
		}
	}

	// stm << "Pattern = " << grpList << "<br>" << endl;
	stm << "<pre>";
	stm << "<b>O = Open Newsgroup. Posts go directly to newsgroup." << endl
		<< "M = Moderated Newsgroup. Posts go to moderator for "
		<< "processing.</b>" << endl << endl;

	int n = 0;
	const char *p;
	int nArt;
	optCmd.SetPageBeg();
	BOOL bFoundGroup = FALSE;
	BOOL bModerate;
	//cout << grpList << endl;
	Regex ex(grpList.chars());
	while((p = lst->GetLine(ex,&nArt,&bModerate)))
	{
		if(++n < grp.m_opt.GetPageBeg())
			continue;

		if(nEnd != 0 && n >= nEnd)
			break;

		if(nArt < 0)
		{
			// That is group of groups
			stm.form("   %5d ", -nArt);
			stm << "<a href=\"" << NEWSBIN
				<<"/wwwnews?"
				<< optCmd << URLText(p) << ".*\"><B>" << HTMLText(p)
				<< ".*</B></a>"
				<< endl;
		}
		else
		{
			stm.form(" %c %5d ", bModerate?'M':'O', nArt);
			stm << "<a href=\"" << NEWSBIN
				<<"/wwwnews?"
				<< optCmd << URLText(p) << "\">" << HTMLText(p) << "</a>"
				<< endl;
		}
		bFoundGroup = TRUE;
	}

	if(!bFoundGroup)
		stm << NO_GROUP_MSG << "</pre>" << endl;
	else
		stm << "</pre>" << endl;

	optCmd.SetPageBeg(nEnd);
	if(nEnd != 0 && n >= nEnd)
	{
		stm << "<a href=" << NEWSBIN << "/wwwnews?"
			<< optCmd 
			<< URLText(grp.m_stgGroupList) << ">"
			<< "<img src=/drn/images/next.gif alt=\"Next\""
			<< "align=center border=0>"
			<< "</a><br>" << endl;
	}

	stm << "<p>" << DRN_NOTICE << "</p>" << endl;

	while(tmplFile.NextLine())
	{
		q = tmplFile.GetLine();
		stm << q << endl;
	}

	delete lst;
	return stm;
}

//----------------------------------------------------------------------------//
ListNewsGroup::ListNewsGroup (NewsGroupList * grp)
	: m_status(&cout)
{
	m_grp = grp;
	m_pGroupShare = NULL;
	m_nGrp = 0;
}

ListNewsGroup::~ListNewsGroup ()
{
	delete m_pGroupShare;
}

int ListNewsGroup::GetNumArt (const char * q)
{
	char * p = (char *)q;
	if(isspace(*p))
		*p++ = 0;

	//
	// Skip blanks and that is end article number
	//
	while(*p && isspace(*p)) p++;
	int n = atoi(p);

	//
	// Skip field and that is begin article number
	//
	while(*p && !isspace(*p)) p++;
	while(*p && isspace(*p)) p++;
	n = n - atoi(p);
	if(n < 0)
		n = 0;
	else
		n++;

	return n;
}

BOOL ListNewsGroup::IsModerate (const char * q)
{
	const char * p = q;
	if(isspace(*p))
		p++;

	//
	// Skip field and that is end article number
	//
	while(*p && isspace(*p)) p++;
	while(*p && !isspace(*p)) p++;

	//
	// Skip field and that is begin article number
	//
	while(*p && isspace(*p)) p++;
	while(*p && !isspace(*p)) p++;

	//
	// Skip blank and that is moderate information
	while(*p && isspace(*p)) p++;

	return (*p == 'm');
}

char *
ListNewsGroup::GetGroup (ZString & stg)
{
	char * p = m_pGroupShare->GetLine(stg);
	if(p == NULL)
		return p;

	if((++m_nGrp % 100) == 0)
		m_status("Scanned %d groups", m_nGrp);
	return p;
}

//----------------------------------------------------------------------------//

BOOL ListNewsGroupL::open ()
{
	m_pGroupShare = new NewsGroupShare();
	return m_pGroupShare->BeginList();
}

const char * ListNewsGroupL::GetLine (Regex & exp, int * pn, BOOL * pModerate)
{
	ZString tmpStg;
	char * p;
	while((p = GetGroup(m_stg)))
	{
		if(m_stg.length() == 0)
			continue;

		tmpStg = m_stg;
		tmpStg.downcase();
		if(tmpStg.matches(exp))
		{
			*pModerate = IsModerate(p);
			*pn = GetNumArt(p);
			return m_stg.chars();
		}
	}

	return NULL;
}

//----------------------------------------------------------------------------//

BOOL ListNewsGroupG::open ()
{
	m_pGroupShare = new NewsGroupShare();
	return m_pGroupShare->BeginList();
}

const char * ListNewsGroupG::GetLine (Regex & exp, int * pn, BOOL * pModerate)
{
	const char *p;
	const char *q = NULL;
	if(m_bSav)
	{
		m_bSav = FALSE;
		q = (*m_pstgSav).chars();
		p = NULL;
	}
	else
	{
		ZString tmpStg;
		if(m_pstgIn == NULL)
			m_pstgIn = new ZString;
		for(;;)
		{
			if((p = GetGroup(*m_pstgIn)) == NULL)
			{
				//
				// No more newsgroup
				//
				if(m_nGroup == 0)
				{
					q = NULL;					// done
					break;
				}

				delete m_pstgIn;
				m_pstgIn = m_pstgGroup;		// group of newsgroup left
				m_pstgGroup = NULL;
				break;
			}

			if(m_pstgIn->length() == 0)
				continue;					// no name?? - just continue

			//
			// Got more newsgroup
			//
			q = (*m_pstgIn).chars();

			tmpStg = *m_pstgIn;
			tmpStg.downcase();

			//
			// Match with pattern
			//
			ZString ss = tmpStg.at(exp);
			if(ss.length() == 0)			// not match - next
				continue;

			//
			// Match
			//
			//cout << ss << "==" << *m_pstgIn << endl;
			if(ss.length() == tmpStg.length())		// exact match
			{
				//
				// This is a newsgroup
				//
				if(m_nGroup == 0)				// Not counting group of groups
					break;						// just send that

				//
				// Output Group of Newsgroup first - save this newsgroup
				//
				m_bSav = TRUE;
				delete m_pstgSav;
				m_pstgSav = m_pstgIn;
				*m_pstgSav += " ";
				*m_pstgSav += p;

				m_pstgIn = m_pstgGroup;			// Need to put group somewhere
				m_pstgGroup = NULL;				// empty groups
				break;
			}

			if(!m_bTrailDot && tmpStg[ss.length()] != '.')
				continue;

			//
			// Not exact match -> Group of newsgroups
			//
			if(m_nGroup == 0)
			{
				m_nGroup = 1;
				//
				// first one
				//
				delete m_pstg1Group;
				m_pstg1Group = m_pstgIn;		// save the whole IN string 
				*m_pstg1Group += " ";
				*m_pstg1Group += p;
				delete m_pstgGroup;				// if only 1 group, need numart
				m_pstgGroup = new ZString(tmpStg.chars(), ss.length());

				m_pstgIn = new ZString;
				continue;
			}

			//
			// Already counting groups
			//
			//cout << ss << " old " << *m_pstgGroup << endl;
			if(strcasecmp(ss.chars(), m_pstgGroup->chars()) == 0)
			{
				//
				// Another one
				//
				m_nGroup++;
				continue;
			}

			//
			// New group of newsgroups
			//
			ZString * pstg = m_pstgIn;		// Temp
			m_pstgIn = m_pstg1Group;
			m_pstg1Group = pstg;			// save it 
			*m_pstg1Group += " ";
			*m_pstg1Group += p;

			if(m_nGroup == 1)
			{
				//
				// Last one is just one
				//
				m_pstgGroup = new ZString(tmpStg.chars(), ss.length());
				q = (*m_pstgIn).chars();
				for(p = q; *p&&!isspace(*p); p++);
				*pModerate = IsModerate(p);
				*pn = GetNumArt(p);
				return q;
			}

			//
			// Last group has more than one newsgroup
			//
			delete m_pstgIn;
			m_pstgIn = m_pstgGroup;
			m_pstgGroup = new ZString(ss);
			break;
		}
	}

	if(m_nGroup)
	{
		*pn = m_nGroup;
		m_nGroup = m_pstgGroup?1:0;
		if(*pn > 1)
		{
			*pn = - *pn;			// negative implies group of newsgroups
			return (*m_pstgIn).chars();
		}

		// only 1 group - make it a newsgroup instead
		q = (*m_pstg1Group).chars();
		p = NULL;
	}

	if(q)
	{
		if(p == NULL)
			for(p = q; *p && !isspace(*p); p++) ;
		*pModerate = IsModerate(p);
		*pn = GetNumArt(p);
		return q;
	}

	return NULL;
}
