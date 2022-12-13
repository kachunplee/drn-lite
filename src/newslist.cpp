#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>
#include <string.h>
#include <string>

#include "def.h"
#include "dgauth.h"
#include "newslib.h"

#include "advert.h"
#include "tmplerr.h"
#include "userinfo.h"

#ifdef GROUP_SUB
#include "newsacc.h"
#endif

#include "newslist.h"
#include "newsmidx.h"

ostream& operator << (ostream& stm, NewsArticleList& list)
{
	TemplateError tmplerr;

	ZString stgGroup = list.m_stgGroup;
	if(stgGroup.lastchar() == '/')
		stgGroup.DelLast();

#ifdef GROUP_SUB
	NewsAccess newsAccess;
	if(!newsAccess.IsGroupSubscribe(stgGroup))
	{
		tmplerr.OutError(stm, NO_GROUP_MSG);
		return stm;
	}
#endif

	ZString tmplName = DRNTMPLDIR;
	tmplName += "/list.htm";
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

	char cSort;
	switch(list.m_opt.GetSortType())
	{
	case SORT_DATE:
	case SORT_X_DATE:
	case SORT_SUM_DATE:
		cSort = 'D';
		break;

	case SORT_SUBJECT:
	case SORT_SUMMARY:
		cSort = 'S';
		break;

	case SORT_AUTHOR:
	case SORT_SUM_AUTHOR:
		cSort = 'A';
		break;

	case SORT_THREAD:
		cSort = 'T';
		break;

	case SORT_R_DATE:
	case SORT_R_SUM_DATE:
	default:
		cSort = 'd';
		break;
	}

	// Scan for <!--pathlink drn=list -->
	BOOL bDspList = FALSE;
	char * p;
	do
	{
		p = tmplFile.GetLine();

		if(strncmp(p, "<head>", 6) == 0)
		{
			stm << p << endl
				<< "<script language=javascript>" << endl
				<< "<!--" << endl
				<< "var Newsgoup = '" << stgGroup.c_str() << "';" << endl
				<< "var sortType = '" << cSort << "';" << endl
				<< "// -->" << endl
				<< "</script>" << endl
			;
			continue;
		}

		if(strncmp(p, "<!--pathlink drn=list -->", 25) == 0)
		{
			bDspList = TRUE;
			break;
		}

		stm << p << endl;
	} while(tmplFile.NextLine());

	if(bDspList)
	{
		if(list.m_optCmd.GetPageBeg() < 0)
			list.m_optCmd.SetPageBeg();

		static struct TB_DEF {
			int type;
			char * fname;
			char * desc;
			char * cord;
			char * image;
		} tb_def [] = {
		  { SORT_THREAD, "SortThread", "Thread Subject", "15,50,87,67", "listtool_T.gif" },
		  { SORT_SUM_DATE, "SortDate", "Date", "93,50,171,67", "listtool_D.gif" },
		  { SORT_R_SUM_DATE, "SortRevDate", "Rev Date", "177,50,262,67", "listtool_d.gif" },
/*
		  { SORT_ARTNUM, "SortArtNum", "Art Num", "84,19,123,43", "listtool_aa.gif" },
		  { SORT_R_ARTNUM, "SortRevArtNum", "Reverse Art Num", "84,0,123,18", "listtool_da.gif" },
*/
		  { SORT_SUMMARY, "SortSubject", "Subject", "267,50,341,67", "listtool_S.gif" },
		  { SORT_SUM_AUTHOR, "SortAuthor", "Author", "347,50,417,67", "listtool_A.gif" },
		};

		stm
			<< "<script language=javascript>" << endl
			<< "<!--" << endl
			<< "function DrnHelp()" << endl
			<< "{" << endl
			<< "    location.href = \"" << DRN_HELP << "\";" << endl
			<< "}" << endl
			<< "function Preference()" << endl
			<< "{" << endl
			<< "    location.href = \"" << NEWSBIN << "/preference\";" << endl
			<< "}" << endl
		;

		for(unsigned n = 0; n < sizeof(tb_def)/sizeof(TB_DEF); n++)
		{ 
			stm 
				<< "function " << tb_def[n].fname << "()" << endl
				<< "{" << endl
			;
//			if(list.m_opt.GetSortType() != tb_def[n].type)
//			{
				list.m_optCmd.SetSortType(tb_def[n].type);
				stm
					<< "    location.href = \"" 
					<< NEWSBIN << "/wwwnews?"
					<< list.m_optCmd << URLText(list.m_stgGroup)
					<< "\";" << endl
				;
//			}
			stm
				<< "}" << endl
			;
		}

		list.m_optCmd.SetSortType(list.m_opt.GetSortType());

		list.m_optCmd.SetBinary(list.m_opt.IsBinary() == FALSE);
		stm
			<< "function ListBinOnly()" << endl
			<< "{" << endl
			<< "    location.href = \""
			<< NEWSBIN << "/wwwnews?"
			<< list.m_optCmd << URLText(list.m_stgGroup)
			<< "\";" << endl
			<< "}" << endl
		;
		list.m_optCmd.SetBinary(list.m_opt.IsBinary());

		stm
			<< "function NewsPost()" << endl
			<< "{" << endl
			<< "    location.href = \""
			<< NEWSBIN << "/wwwpost?" << URLText(list.m_stgGroup)
			<< "\";" << endl
			<< "}" << endl
			<< "// --> " << endl
			<< "</script>" << endl << endl
		;

//		list.OutBegin(stm, pToolImg);
		list.m_pctxRoot = NULL;

#ifdef USERINFO
		const char * pUser = gUserInfo.GetUserName();
		const char * pHostAddr = gUserInfo.GetHostAddr();
		const char * pHostDomain = gUserInfo.GetHostDomain();
		char * pPasswd = NULL;

		if(pHostDomain && *pHostDomain != 0)
		{
			// ISP connection
			if(pUser)
				list.m_pctxRoot = new MD5_CTX;
		}
		else
			list.m_pctxRoot = new MD5_CTX;
		if(list.m_pctxRoot)
		{
			const char * pDBName = DBNAME;
			pPasswd = gethashpwd(pUser, pDBName);
			MD5root(list.m_pctxRoot, pUser, pPasswd, pHostAddr);
		}
#endif

		int n = list.GetIndex(&stm, list.m_opt.GetSortType());
		if(n == -2)
		{
			stm << "<h3>Unknown SORT Option</h3>" << endl;
			goto End;
		}

		if(n <= 0)
		{
			stm << "<h3>No article in group</h3>" << endl;
			goto End;
		}

#ifdef REGULAR
		list.m_nMinArtNum = newsAccess.GetMinArtNum(stgGroup);
#endif

		//
		// - Beg => Goto
		//
		ARTNUM nArt = list.Goto(-list.m_opt.GetPageBeg(),
			list.m_opt.GetFindOff());
		if(nArt < 0)
			stm << "<h3>Cannot find article</h3>" << endl;
		else
		{
			switch(list.OutArticles(stm, nArt))
			{
			case -1:
				stm << "<h3>Article range out of bound</h3>" << endl;
				break;

			case -2:
				stm << "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0;\">"
					<< "<B>Article List Index needs to be rebuild - if screen does not refresh itself, please hit refresh</B>"
					<< endl;
				break;

			default:
				list.OutEnd(stm);
			}
		}
	}

End:
	stm << "<p>" << DRN_NOTICE << "</p>" << endl;

	while(tmplFile.NextLine())
	{
		p = tmplFile.GetLine();
		stm << p << endl;
	}

	return stm;
}

//
//
//
NewsArticleList::NewsArticleList (ZString& stg, NewsOption& opt)
	: m_stgGroup(stg), m_opt(opt), m_optCmd(opt)
{
	m_nMinArtNum = 0;
	m_nArticle = 0;
	m_pctxRoot = NULL;
	m_pIndex = NULL;

	m_dirGroup = m_stgGroup;
	for(unsigned int i = 0; i < m_dirGroup.length(); i++)
	{ if(m_dirGroup[i] == '.') m_dirGroup[i] = '/'; }
}

NewsArticleList::~NewsArticleList ()
{
	delete m_pctxRoot;
	delete m_pIndex;
}

//
int
NewsArticleList::GetIndex (ostream * pstm, int sortType)
{
	m_idxSortType = sortType;

	m_pIndex = MakeIndex(m_stgGroup, m_dirGroup, pstm,
					sortType, &m_nDir, m_nMinArtNum, m_opt.IsBinCol());
	if(m_pIndex == NULL)
		return -2;

	return (m_nArticle = m_pIndex->Fetch());
}

//
// Goto ARTNUM offset by n
// return 0 if not find
//		else set PageBeg
ARTNUM
NewsArticleList::Goto (ARTNUM nArt, int n)
{
	if(nArt <= 0)
		return 0;

	int r = m_pIndex->Find(nArt);
	if(r < 0)
	 	return -1;

	r = (m_nDir > 0) ? r+1 : m_nArticle-r;
	int nGet = GetPageLen()-1;
	if(n < 0) n = 0;
	r -= (nGet>0 && nGet<n) ? nGet : n;
	m_opt.SetPageBeg((r>0)?r:1);
	return nArt;
}

//
//
int NewsArticleList::GetPageLen ()
{
	return (m_opt.GetPageLen()<=0) ? m_nArticle : m_opt.GetPageLen();
}

//
//
int NewsArticleList::OutArticles (ostream& stm, ARTNUM nArt)
{
	int nPageLen = GetPageLen();
	int nGet = m_pIndex->Set(m_opt.GetPageBeg()-1, nPageLen, m_nDir);
	if(nGet <= 0)
		return -1;

	OutLink(stm);

	ZString stg;
	ARTNUM artno;
	char * p;
	int n = nGet;

	for(int i=(m_nDir>0)?0:(nGet-1); n > 0; n--, i+=m_nDir)
	{
		if(!m_pIndex->Get(i, stg))
		{
			m_pIndex->Bad();
			return -2;
		}

		artno = atoi(stg);					// 1st = Article Number
		if(artno == 0)
		{
			// missing article no., something wrong with the index file
			m_pIndex->Bad();
			return -2;
		}
		p = strchr(stg,'\t');
		if(p == NULL)
		{
			// no tab, something wrong with the index file
			m_pIndex->Bad();
			return -2;
		}
		OutArticle(stm, stg, i, nPageLen, nArt==artno);
	}

	return (nGet - n + m_optCmd.GetPageBeg());
}

//----------------------------------------------------------------------
//
void NewsArticleList::OutBegin (ostream& stm, char * pToolImg)
{
}

void NewsArticleList::OutLink (ostream& stm)
{
	OutPageLinks(stm);
	stm << "<pre>";

  {
	const char * p = m_stgGroup;
	const char * q = strrchr(p, '.');
	if(q == NULL)
		q = p;
	else
		q++;
	ZString sstg = m_stgGroup.substr(0, q-p);
	q = strchr(p, '.');
	ZString sstg1 = m_stgGroup.substr(0, q-p+1);
	m_optCmd.SetPageBeg();
/*
	if(sstg1.length() < sstg.length())
		stm << "<img src=/drn/images/drnheir.gif>"
			<< "<B><a href=\"" << NEWSBIN << "/wwwnews?"
			<< m_optCmd << URLText(sstg1) << "*\">"
			<< HTMLText(sstg1) << "*</a></B>   "
		;
*/
	stm
		<< "<img src=/drn/images/drnheir.gif>"
		<< "<B><a href=\"" << NEWSBIN << "/wwwnews?"
		<< m_optCmd << URLText(sstg) << "*\">"
		<< HTMLText(sstg) << "*</a> "
	;

#ifdef WWWNEWSS
	stm
		<< "<a href=\"" << NEWSBIN << "/wwwnewss?"
		<< URLText(m_stgGroup) << "\">"
		<< "<img src=/drn/images/newsbn.gif border=0></a> "
	;
#endif

	m_optCmd.SetPageBeg(m_opt.GetPageBeg());
  }

	stm
		<< " </b>" << "<img src=\"/drn/images/drnlegnd.gif\" "
		<< "align=\"top\">"
		<< endl << endl
	;
}

void NewsArticleList::OutEnd (ostream& stm)
{
	stm << "</pre>";
	OutPageLinks(stm);
}

#define MAX_DSPPAGE 20

void NewsArticleList::OutPageLinks (ostream& stm)
{
	if(m_opt.GetFindOff() < 0)
		return;

	stm
		<< "<br>" << endl
		<< "<strong><font face=\"Arial\" size=\"2\">"
	;
	int nPageLen = GetPageLen();
	int nCurPage = m_opt.GetPageBeg() / nPageLen;
	BOOL bSpace = FALSE;
	if(nCurPage >= MAX_DSPPAGE)
	{
		m_optCmd.SetPageBeg(1);
		stm
			<< "<a href=\"" << NEWSBIN << "/wwwnews?"
			<< m_optCmd << URLText(m_stgGroup)
			<< "\" alt=\"First Page\"><img src=/drn/images/start.gif "
			<< "border=0 align=absbottom></a>"
			<< "&nbsp;&nbsp;"
		;
		bSpace = TRUE;
	}
	int nStartPage = (nCurPage / MAX_DSPPAGE) * MAX_DSPPAGE;
	if(nStartPage >= (MAX_DSPPAGE * 2))
	{
		m_optCmd.SetPageBeg((nStartPage - MAX_DSPPAGE) * nPageLen + 1);
		stm
			<< "<a href=\"" << NEWSBIN << "/wwwnews?"
			<< m_optCmd << URLText(m_stgGroup)
			<< "\" alt=\"Back To Previous Page\">"
			<< "<img src=/drn/images/back.gif "
			<< "border=0 align=absbottom></a>"
			<< "&nbsp;&nbsp;"
		;
		bSpace = TRUE;
	}
	if(bSpace)
		stm << "&nbsp;&nbsp;&nbsp;";
	int i, nOffset;
	BOOL bPage = FALSE;
	for(i = 0; i < MAX_DSPPAGE; i++)
	{
		nOffset = (nStartPage + i) * nPageLen + 1;
		if(nOffset > m_nArticle)
			break;
		if(bPage)
		{
			stm << " . ";
		}
		if((nStartPage + i) != nCurPage)
		{
			m_optCmd.SetPageBeg(nOffset);
			stm
				<< "<a href=\"" << NEWSBIN << "/wwwnews?"
				<< m_optCmd << URLText(m_stgGroup)
				<< "\" alt=\"Page " << nStartPage + i + 1
				<< "\">" << nStartPage + i + 1 << "</a>"
				<< endl
			;
		}
		else
		{
			stm
				<< "<font face=Arial size=4>"
				<< nStartPage + i + 1
				<< "</font>"
			;
		}
		bPage = TRUE;
	}
	if(i >= MAX_DSPPAGE)
	{
		nOffset = (nStartPage + i) * nPageLen + 1;
		if(nOffset < m_nArticle)
		{
			m_optCmd.SetPageBeg(nOffset);
			stm
				<< "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
				<< "<a href=\"" << NEWSBIN << "/wwwnews?"
				<< m_optCmd << URLText(m_stgGroup)
				<< "\" alt=\"More...\">"
				<< "<img src=/drn/images/more.gif border=0 align=top>"
				<< "</a>"
				<< endl
			;
		}
	}

	stm << "</font></strong><p>" << endl;
}

void NewsArticleList::OutArticle (ostream& stm, const char * s,
	int idx, int nPageLen, BOOL bBold)
{
	char buf[strlen(s)+1];
	char * p = buf;
	strcpy(p, s);

	char * szArtNum = p;
	while(*p && *p != '\t') p++;
	*p++ = 0;

	char * q = p;								// 2nd = Subject
	while(*p && *p != '\t') p++;	
	*p++ = 0;
	NewsSubject Subject(q);

	q = p;										// 3rd = Author
	while(*p && *p != '\t') p++;
	*p++ = 0;
	ZString Author;
	INMailName(q).RealName(Author);

	q = p;										// 4th = Date
	while(*p && *p != '\t') p++;
	*p++ = 0;
	INDateTime dt(q);

	for(int i = 4; i < 6; i++)					// 7th = number of line
	{
		while(*p && *p != '\t') p++;
		p++;
	}

	int nByte = atoi(p);
	while(*p && *p != '\t') p++;

	int nLine = atoi(++p);

	ZString stgType;
	ZString stgTip;
	BOOL bBinary = FALSE;
	int iFile = -1;
	if(nLine > 50)
	{
		switch(Subject.GetFileExt(&iFile))
		{
		case EXT_PHOTO:
			stgType = "newsphoto.gif";
			stgTip = "Click to view image";
			bBinary = TRUE;
			break;

		case EXT_VIDEO:
			stgType = "newsvideo.gif";
			stgTip = "Click to download";
			bBinary = TRUE;
			break;

		case EXT_BINARIES:
			stgType = "newsbin.gif";
			stgTip = "Click to download";
			bBinary = TRUE;
			break;

		case EXT_SOUND:
			stgType = "newssound.gif";
			stgTip = "Click to download";
			bBinary = TRUE;
			break;
		}
	}

	BOOL bCheck = FALSE;
	int nMulti = m_pIndex->GetMulti(idx);
	int nPart = nMulti & NLINE_NPART;

	if((nMulti & NLINE_MULTI) != 0)
	{
		/* check if multi-part is completed */
		bCheck = (nPart > 1) && ((nMulti & NLINE_COMPLETED) != 0);
		nMulti &= ~NLINE_COMPLETED;
	}

	if((nMulti&NLINE_BINARC) != 0 && nPart <= 1 && bBinary)
		nPart = 1;

	/* set ICON type to binaries if multi-part */
	if(stgType.length() == 0
			&& nLine > 50
			&& ((nMulti > NLINE_MULTI)
				|| ((nMulti&NLINE_MULTI) == 0 && Subject.IsMultiPart()))
		)
	{
		stgType = "newsbin.gif";
		stgTip = "Click to download";
		bBinary = TRUE;
	}

	if(m_opt.IsBinary() && !bBinary)
		return;

	BOOL bArc = (nPart > 1 && (nMulti&NLINE_BINARC) != 0);

	if(bBold) stm << "<strong>";
	Author.gsub("&", "+");
	Author.gsub("<", "(");
	Author.gsub(">", ")");

	//
	// Output date and author
	//
	stm.form("%02d/%02d %-12.12s ",
		dt.time().tm_mon+1, dt.time().tm_mday,
		Author.chars());

	//
	// Make bytes string
	// and need to calculate leading space
	//
	int bytes = m_pIndex->GetSize(idx);
	if(bytes == 0) bytes = nByte;
	const int maxDigit = 6;
	int ndigit;
	char szByte [maxDigit+1];

	if(bArc)
	{
		if(nPart < 10000)
			ndigit = sprintf(szByte, "%dA", nPart);
		else
			ndigit = sprintf(szByte, "*****A");
	}
	else if(bytes < 1000)
	{
		ndigit = sprintf(szByte, "%dB", bytes);
	}
	else if(bytes < 1000000)
	{
		ndigit = sprintf(szByte, "%d.%0dK",
			bytes/1000, (bytes+500)/100%10);
	}
	else if(bytes < 1000000000)
	{
		ndigit = sprintf(szByte, "%d.%0dM",
			bytes/1000000, (bytes+500000)/100000%10);
	}
	else
	{
		ndigit = sprintf(szByte, "******");
	}
	
	while(ndigit++ < maxDigit)
		stm << ' ';

	char * pDigest = NULL;
	if(!bArc && stgType.length() > 0)
	{
#ifdef DIGESTPWD
		if(m_pctxRoot)
		{
			m_ctxCmd = *m_pctxRoot;
			unsigned char md[MDBUFLEN];
			MD5Update(&m_ctxCmd, (unsigned char *)NEWSBIN,
				strlen(NEWSBIN));
			MD5Update(&m_ctxCmd, (unsigned char *)"/wwwdecode?",
				11);
			MD5Update(&m_ctxCmd,
				(unsigned char *)m_stgGroup.chars(),
				m_stgGroup.length());
			MD5Update(&m_ctxCmd, (unsigned char *)"/", 1);
			MD5cmd(&m_ctxCmd, md, szArtNum);
			md_digest(md, m_Digest, gUserInfo.GetUserName());
			pDigest = m_Digest;
		}
#endif

		stm << "<a href=\"" << DECODE_SERVER;
		if(pDigest)
			stm << "/.." << pDigest;
		stm << NEWSBIN << "/wwwdecode?" << URLText(m_stgGroup)
			<< "/" << szArtNum << "\">";

		stm << szByte << "</a> ";

		stm << "<a href=\"" << DECODE_SERVER;
		if(pDigest)
			stm << "/.." << pDigest;
		stm << NEWSBIN << "/wwwdecode?" << URLText(m_stgGroup)
			<< "/" << szArtNum << "\">";
	}
	else
	{
		m_pIndex->OutURL(stm, idx, m_stgGroup, szArtNum, nPageLen);
		stm << szByte << "</a> ";

		m_pIndex->OutURL(stm, idx, m_stgGroup, szArtNum, nPageLen);
	}

	if(bArc)
	{
		char buf[20];
		sprintf(buf, "%d", nPart);
		stgTip = buf;
		stgTip += " articles";
	}
	else if(bBinary && !bCheck && (nMulti&NLINE_MULTI) != 0)
	{
		char buf[20];
		sprintf(buf, "%d", nPart);
		stgTip = buf;
		stgTip += " part(s) available";
	}

	stm << "<img src=/drn/images/" 
		<< (bCheck ? "drncheck.gif"
			: (bArc ? "drnplus.gif"
			  : (bBinary && (nMulti&NLINE_MULTI)) ? "drnpart.gif"
					: "drnblank.gif"))
		<< " align=bottom border=0";
	
	if(bBinary)
	{
		stm << " alt=\"" << stgTip << "\"";
	}
	stm	<< ">";

	if(stgType.length() > 0)
	{
		stm  << "<img src=\"/drn/images/"
			<< stgType << "\" border=0 " << "alt=\"" << stgTip
			<< "\"></a> ";
	}
	else
	{
		stm << "<img src=\"/drn/images/newstext.gif\" border=0 "
			<< "alt=\"Click to read text article\"></a> ";
		;
	}

	m_pIndex->OutSubject(stm, idx, m_stgGroup, szArtNum, Subject,
		nPageLen, bArc?iFile:-1);

	if(bBold)
		stm << "</strong>";
	stm << endl;
}
