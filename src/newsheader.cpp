#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>

#include "def.h"

#include "advert.h"
#include "tmplerr.h"
#include "userinfo.h"

#ifdef GROUP_SUB
#include "newsacc.h"
#endif

#include "newsmime.h"
#include "newslist.h"
#include "newsheader.h"

#include "artspool.h"

ostream& operator << (ostream& stm, NewsHeader& header)
{
	TemplateError tmplerr;
	if(header.m_stgArticle[0] == '<' && header.m_stgArticle.lastchar() == '>')
	{
		//
		// Look up history file to translate ID into a file name
		//
		tmplerr.OutError(stm, "Message ID not support yet");
		return stm;
	}

	ZString stgGroup = header.m_stgArticle;
	header.m_nArticleNo = 0;
	const char * pGroup = stgGroup;
	const char * szArtno = strrchr(pGroup, '/');
	if(szArtno)
	{
		const char * pArt = ++szArtno;
		for(; *pArt; pArt++) { if(!isdigit(*pArt)) break; }
		if(*pArt == 0)
		{
			header.m_nArticleNo = szArtno - pGroup;
			stgGroup[header.m_nArticleNo-1] = 0;
		}
	}

	if(stgGroup.length() == 0)
	{
		tmplerr.OutError(stm, NO_GROUP_MSG);
		return stm;
	}

#ifdef GROUP_SUB
	NewsAccess newsAccess;
	if(!newsAccess.IsGroupSubscribe(stgGroup))
	{
		tmplerr.OutError(stm, NO_GROUP_MSG);
		return stm;
	}
#endif

	ZString TmplName = DRNTMPLDIR;
	TmplName += "/header.htm";
	Zifstream tmplFile(TmplName, ios::nocreate|ios::in);
	if(!tmplFile)
	{
		ZString err = "<h3>Template file " + TmplName;
		err += " is not found</h3>";
		tmplerr.OutError(stm, err.chars());
		return stm;
	}

	if(!tmplFile.good())
	{
		ZString err = "<h3>Template file " + TmplName;
		err += " is empty</h3>";
		tmplerr.OutError(stm, err.chars());
		return stm;
	}

	// Scan for <!--pathlink drn=header -->
	BOOL bDspHeader = FALSE;
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
				<< "var ArtNum = '" << szArtno << "';" << endl
				<< "// -->" << endl
				<< "</script>" << endl
			;
			continue;
		}

		if(strncmp(p, "<!--pathlink drn=header -->", 27) == 0)
		{
			bDspHeader = TRUE;
			break;
		}

		stm << p << endl;
	} while(tmplFile.NextLine());

	if(bDspHeader)
	{
		ArticleSpool artspool(stgGroup, szArtno);
		if(!artspool.good())
		{
			header.OutToolbar(stm, stgGroup.chars(), NULL);
			stm << "<h3>Article " << header.m_stgArticle << " not found</h3>"
				<< endl << "This article may have been expired" << endl;
			header.OutEnd(stm, stgGroup, tmplFile);
			return stm;
		}
		artspool.NoFile();

		// Scan headers
		ZString stgHeaders, stgMessageID, stgTmp;
		char * q;
		while((p = artspool.GetLine()))
		{
			if(*p == 0 || *p == '\r')
				break;
			
			q = strchr(p, ' ');
			if(q)
				*q++ = 0;

			if(strncasecmp(p, "from:", 5) == 0)
			{
				stgTmp = header.pr_from(stm, q);
				stgHeaders += stgTmp;
				continue;
			}
			else if(strncasecmp(p, "newsgroups:", 11) == 0)
			{
				stgTmp = header.pr_newsgroups(stm, q);
				stgHeaders += stgTmp;
				continue;
			}
			else if(strncasecmp(p, "Path:", 5) == 0)
			{
				continue;			// skip the path header
			}
			else if(strncasecmp(p, "Message-ID:", 11) == 0)
			{
				stgMessageID = q;
			}
			else if(q == NULL)
			{
				q = p;
				p = NULL;
			}
			if(p)
			{
				stgHeaders += "<b>";
				stgHeaders += p;
				stgHeaders += " </b>";
			}
			HTMLText h(q);
			stgHeaders += h.GetText();
			stgHeaders += "<br>\n";
		}

		header.OutToolbar(stm, stgGroup.chars(), stgMessageID.chars());

		//
		// Display headers
		//
		stm << "<font size=-1 face=\"Arial\">" << endl
			<< "<TITLE>Header of " << header.m_stgArticle << "</TITLE><br>"
			<< endl
			<< stgHeaders
			<< "</font><p>" << endl;
	}
	else
	{
		header.OutToolbar(stm, stgGroup.chars(), NULL);
	}

	header.OutEnd(stm, stgGroup, tmplFile);
	return stm;
}

void NewsHeader::OutToolbar(ostream& stm, const char * pGroup,
	const char * pMessageID)
{
	ZString stgGroup = pGroup;

	stm
		<< "<script language=javascript>" << endl
		<< "<!--" << endl;
	if(pMessageID)
		stm << "var MessageID = \"" << pMessageID << "\";" << endl;

	stm << "function NewsPost()" << endl
		<< "{" << endl
		<< "    location.href = \"" << NEWSBIN << "/wwwpost?"
		<< URLText(m_stgArticle) << "\";" << endl
		<< "}" << endl
		<< "// -->" << endl
		<< "</script>" << endl
	;
}

void NewsHeader::OutEnd (ostream& stm, ZString &, Zifstream& tmplFile)
{
	stm << "<p>" << DRN_NOTICE << "</p>" << endl;

	char * p;
	while(tmplFile.NextLine())
	{
		p = tmplFile.GetLine();
		stm << p << endl;
	}
}

ZString NewsHeader::pr_from (ostream& stm, char * pFrom)
{
	ZString MailAddr;
	INMailName(pFrom).EMailAddress(MailAddr);
	ZString stgFrom = "<b>From: </b>";
	if( MailAddr.contains('@') && MailAddr.contains('.') )
	{
		stgFrom += "<a href=\"mailto:";
		URLText u(MailAddr.chars());
		stgFrom += u.GetText();
		stgFrom += "\">";
		HTMLText h(pFrom);
		stgFrom += h.GetText();
		stgFrom += "</a><br>\n";
	}
	else
	{
		HTMLText h(pFrom);
		stgFrom += h.GetText();
		stgFrom += "<br>\n";
	}
	return stgFrom;
}

ZString NewsHeader::pr_newsgroups (ostream& stm, char * p)
{
	ZString stgGroups = "<b>Newsgroups: </b>";
	int n;
	char c;
	for(int i = 1 ;; i++)
	{
		n = strcspn(p, ",");
		c = p[n];
		p[n] = 0;
		if(i > 1)
			stgGroups += ", ";
		stgGroups += "<a href=\"";
		stgGroups += NEWSBIN;
		stgGroups += "/wwwnews?";
		URLText u(p);
		stgGroups += u.GetText();
		stgGroups += "\">";
		HTMLText h(p);
		stgGroups += h.GetText();
		stgGroups += "</a>";
		if(c == 0)
			break;
		p[n] = c;
		p += n+1;
	}
	stgGroups += "<br>\n";
	return stgGroups;
}
