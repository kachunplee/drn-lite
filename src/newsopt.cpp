#include <stdio.h>
#include <sys/stat.h>
#include "def.h"
#include "string.h"
#include "userinfo.h"

NewsOption::NewsOption (int nP /*=DEF_PAGE*/, int nB /*=DEF_BEGIN*/,
	char cS /*=DEF_SORT*/, BOOL bBinary /*=FALSE*/)
{
	m_nPageLen = m_nDefPage = nP;
	m_nPageBeg = nB;
	m_nFindOff = DEF_FINDOFF;
	m_cSortType = m_cDefSort = cS;
	m_bBinCol = m_bDefBinCol = FALSE;
	m_bBinary = bBinary; m_bStgOK = FALSE;

	m_nAttSplitSize = DEF_ATTSPLITSIZE;
	m_nMinSplitSize = 1000;;
	m_nMaxSplitSize = 15000;
	m_bNoCrossPost = m_bNoXArchive = FALSE;
	m_stgOrganization = DEF_ORGANIZATION;
	m_stgMailDomain = DEF_MAILDOMAIN;
	m_stgDrnServer = DEF_DRNSERVER;
	m_stgWWWRoot = DEF_WWWROOT;
	m_stgNNTPservers = NNTPservers;
	m_stgNNTPauths = NNTPauths;

	struct stat st;
	ZString filename = "../etc/drn.conf";
	if(stat(filename, &st) == -1)
	{
		DMSG(1, "Cannot open $s", filename.chars());
		filename = ETCDIR;
		filename += "/drn.conf";
	} 
	ifstream confFile(filename.chars(), ios::nocreate|ios::in);
	ZString s1,s2;
	int n;
	const char * p;
	while(readline(confFile, s1) > 0)
	{
		if(s1[0] == '#')
			continue;

		n = s1.find('=');
		s2 = s1.substr(n+1, s1.length()-n);
		s1 = s1.substr(0, n);
		n = s2.length();
		if(s2.lastchar() == ';')
			s2.DelLast();
		if((s2[0] == '"' && s2.lastchar() == '"') ||
			(s2[0] == '\'' && s2.lastchar() == '\''))
		{
			// strip the quote
			s2.DelLast();
			s2.del(0, 1);
		}

		if(s2.length() == 0)
			continue;

		if(s1 == "$DefaultSort")
		{
			m_cDefSort = s2[0];
			//
			// To remain backward compatibility
			//
			switch(m_cDefSort)
			{
			case SORT_X_DATE:
				m_cDefSort = SORT_R_SUM_DATE;
				break;
			case SORT_SUBJECT:
				m_cDefSort = SORT_SUMMARY;
				break;
			case SORT_AUTHOR:
				m_cDefSort = SORT_SUM_AUTHOR;
				break;
			}
			m_cSortType = m_cDefSort;
		}
		else if(s1 == "$PageLength")
		{
			p = s2.chars();
			if(isdigit(*p))
				m_nPageLen = m_nDefPage = atoi(p);
		}
		else if(s1 == "$AttachSplitSize")
		{
			p = s2.chars();
			if(isdigit(*p))
				m_nAttSplitSize = atoi(p);;
		}
		else if(s1 == "$MaxSplitSize")
		{
			p = s2.chars();
			if(isdigit(*p))
				m_nMaxSplitSize = atoi(p);;
		}
		else if(s1 == "$Organization")
			m_stgOrganization = s2;
		else if(s1 == "$MailDomain")
			m_stgMailDomain = s2;
		else if(s1 == "$DrnServer")
			m_stgDrnServer = s2;
		else if(s1 == "$WWWRoot")
			m_stgWWWRoot = s2;
		else if(s1 == "$NNTPservers")
			m_stgNNTPservers = s2;
		else if(s1 == "$NNTPauths")
			m_stgNNTPauths = s2;
	}

	NNTPservers = m_stgNNTPservers;
	NNTPauths = m_stgNNTPauths;

	DEF_WWWROOT = m_stgWWWRoot.chars();
	m_stgDrnTmplDir = m_stgWWWRoot + DRN_TMPLDIR;
	DRNTMPLDIR = m_stgDrnTmplDir.chars();
	m_stgUserDir = m_stgWWWRoot + DRN_USERDIR;
	USERDIR = m_stgUserDir.chars();
	m_stgDecodeDir = m_stgWWWRoot + DRN_DECODEDIR;
	DECODEDIR = m_stgDecodeDir.chars();
	m_stgLockDecodeDir = m_stgWWWRoot + DRN_LOCKDECODEDIR;
	LOCKDECODEDIR = m_stgLockDecodeDir.chars();
	m_stgTempDecodeDir = m_stgWWWRoot + DRN_TEMPDECODEDIR;
	TEMPDECODEDIR = m_stgTempDecodeDir.chars();
	m_stgDrnCacheDir = m_stgWWWRoot + DRN_CACHEDIR;
	DRNCACHEDIR = m_stgDrnCacheDir.chars();
	m_stgDrnCacheFile = m_stgWWWRoot + DRN_CACHEFILE;
	DRNCACHEFILE = m_stgDrnCacheFile.chars();
	m_stgDrnCleanCache = m_stgWWWRoot + DRN_CLEANCACHE;
	DRNCLEANCACHE = m_stgDrnCleanCache.chars();

	DRN_SERVER = m_stgDrnServer.chars();
	DECODEDIR_SERVER = m_stgDrnServer.chars();
}
	
void NewsOption::ReadPreference ()
{
	// read the user preference page to find the default sorting option
	//	and display page length
	ZString filename = USERDIR;
	filename += gUserInfo.GetUserName();
	filename += "/preference";
	Zifstream * prefFile = new Zifstream(filename.chars(),
		ios::nocreate|ios::in);
	if(!prefFile || !prefFile->good())
	{
		delete prefFile;
		filename = m_stgWWWRoot + DRN_ETCDIR;
		filename += "preference";
		prefFile = new Zifstream(filename.chars(), ios::nocreate|ios::in);
	}
	if(prefFile && prefFile->good())
	{
		// Scan the preference file to look for sort type & page length
		do
		{
			char * q = prefFile->GetLine();
			if(*q == 0 || *q == '\r')
				break;
			
			if(strncasecmp(q, "sort=", 5) == 0)
			{
				m_cDefSort = q[5];
				//
				// To remain backward compatibility
				//
				switch(m_cDefSort)
				{
				case SORT_X_DATE:
					m_cDefSort = SORT_R_SUM_DATE;
					break;
				case SORT_DATE:
					m_cDefSort = SORT_SUM_DATE;
					break;
				case SORT_SUBJECT:
					m_cDefSort = SORT_SUMMARY;
					break;
				case SORT_AUTHOR:
					m_cDefSort = SORT_SUM_AUTHOR;
					break;
				}
				m_cSortType = m_cDefSort;
			}
			else if(strncasecmp(q, "pagelength=", 11) == 0)
				m_nPageLen = m_nDefPage = atoi(q + 11);
			else if(strncasecmp(q, "bincol=", 7) == 0)
				m_bBinCol = m_bDefBinCol = (strncasecmp(q+7, "yes", 3) == 0);
			else if(strncasecmp(q, "nocrosspost=", 12) == 0)
			{
				if(strncasecmp(q+12, "yes", 3) == 0)
					m_bNoCrossPost = TRUE;
			}
			else if(strncasecmp(q, "fromname=", 9) == 0)
				m_stgFromName = q + 9;
			else if(strncasecmp(q, "fromemail=", 10) == 0)
				m_stgFromEmail = q + 10;
			else if(strncasecmp(q, "organization=", 13) == 0)
				m_stgOrganization = q + 13;
			else if(strncasecmp(q, "replyemail=", 11) == 0)
				m_stgReplyEmail = q + 11;
			else if(strncasecmp(q, "attsplitsize=", 13) == 0)
				m_stgReplyEmail = atoi(q + 13);
			else if(strncasecmp(q, "noxarchive=", 11) == 0)
			{
				if(strncasecmp(q+11, "yes", 3) == 0)
					m_bNoXArchive = TRUE;
			}
		} while(prefFile->NextLine());

		delete prefFile;
	}
}

NewsOption& NewsOption::operator = (const NewsOption& src)
{
	m_bStgOK = FALSE;
	m_nPageBeg = src.m_nPageBeg;
	m_nPageLen = src.m_nPageLen;
	m_nDefPage = src.m_nDefPage;
	m_cSortType = src.m_cSortType;
	m_cDefSort = src.m_cDefSort;
	m_bBinCol = src.m_bBinCol;
	m_bBinary = FALSE;
	return *this;
}

void NewsOption::SetOptions (const char * p)
{
	while(*p)
	{
		switch(*p++)
		{
		case 's':				// Sort
			SetSortType((*p && *p != ',') ? *p : m_cDefSort);
			break;

		case 'b':				// Begin
			SetPageBeg((*p && *p != ',') ? atoi(p) : DEF_BEGIN);
			break;

		case 'n':				// Len
			SetPageLen((*p && *p != ',') ? atoi(p) : m_nDefPage);
			break;

		case 'o':
			SetFindOff((*p && *p != ',') ? atoi(p) : -1);
			break;

		case 'B':
			SetBinary(TRUE);
			break;

		case 'A':
			SetBinCol(TRUE);
			break;

		case 'F':
			SetBinCol(FALSE);
			break;
		}

		while(*p && *p != ',') p++;
		if(*p == 0)
			break;
		p++;
	}
}

ZString & NewsOption::GetStg ()
{
	if(!m_bStgOK)
	{
		BOOL bNotEmpty = FALSE;

		if(m_cSortType != m_cDefSort)
		{
			m_stg = "s";
			m_stg += m_cSortType;
			bNotEmpty = TRUE;
		}
		else
			m_stg = "";

		char buf[20];

		if(m_nPageBeg != DEF_BEGIN)
		{
			if(bNotEmpty)
				m_stg += ",";
			sprintf(buf, "b%d", m_nPageBeg);
			m_stg += buf;
			bNotEmpty = TRUE;
		}

		if(m_nPageLen != m_nDefPage)
		{
			if(bNotEmpty)
				m_stg += ",";
			sprintf(buf, "n%d", m_nPageLen);
			m_stg += buf;
			bNotEmpty = TRUE;
		}

		if(m_bBinary)
		{
			if(bNotEmpty)
				m_stg += ",";
			m_stg += "B";
			bNotEmpty = TRUE;
		}

		if(m_bBinCol != m_bDefBinCol)
		{
			if(bNotEmpty)
				m_stg += ",";
			m_stg += m_bBinCol?"A":"F";
			bNotEmpty = TRUE;
		}

		m_bStgOK = TRUE;
	}

	return m_stg;
}

ostream& operator <<(ostream& stm, NewsOption& opt)
{
	if(opt.GetStg().length() > 0)
	{
		stm << '-' << opt.GetStg() << '+';
	}
	return stm;
}
