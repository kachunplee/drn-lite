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
#include "newsart.h"

#include "artspool.h"

#ifdef DIGESTPWD
extern char * gethashpwd(const char *, const char *);
extern char * md_digest (unsigned char *, char *, const char *);
extern void MD5root(MD5_CTX *, const char *, const char *, const char *);
extern void MD5cmd(MD5_CTX *, unsigned char *, const char *);
#endif

ostream& operator << (ostream& stm, NewsArticle& art)
{
	TemplateError tmplerr;

	ZString stgGroup = art.m_stgArticle;
	const char * szArtno = NULL;
	if(stgGroup[0] != '<' || stgGroup[art.m_stgArticle.length()-1] != '>')
	{
		szArtno = art.m_stgArticle.chars() + art.m_nArticleNo;
		stgGroup[art.m_nArticleNo-1] = 0;

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
	}

	ZString TmplName = DRNTMPLDIR;
	TmplName += "/article.htm";
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

	// Scan for <!--pathlink drn=article -->
	BOOL bDspArt = FALSE;
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

		if(strncmp(p, "<!--pathlink drn=article -->", 28) == 0)
		{
			bDspArt = TRUE;
			break;
		}

		stm << p << endl;
	} while(tmplFile.NextLine());

	if(bDspArt)
	{
		ArticleSpool artspool(stgGroup, szArtno);
		if(!artspool.good())
		{
			art.OutToolbar(stm, NULL);
			stm << "<h3>Article " << art.m_stgArticle << " not found</h3>"
				<< endl << "This article may have been expired" << endl;
			art.OutEnd(stm, tmplFile);
			return stm;
		}

		art.m_pctxRoot = NULL;
#ifdef USERINFO
		const char * pUser = gUserInfo.GetUserName();
		const char * pHostAddr = gUserInfo.GetHostAddr();
		const char * pHostDomain = gUserInfo.GetHostDomain();
		char * pPasswd = NULL;

		if(pHostDomain && *pHostDomain != 0)
		{
			// ISP connection
			if(pUser)
				art.m_pctxRoot = new MD5_CTX;
		}
		else
		{
			// User connection
			char buf[128];
			snprintf(buf, sizeof(buf), "%s/user/%c/%s.d", USERPID,
				(pUser[0] >= 'a' && pUser[0] <= 'z') ? pUser[0] : '0',
				pUser);
			int fd = open(buf, O_RDWR|O_CREAT|O_EXLOCK, 0644);
			if(fd >= 0)
			{
				int nXfer = (sb.st_size + 512) / 1024;
				if(nXfer < 1) nXfer = 1;
				int nDownload;
				int n = read(fd, (char *) &nDownload, sizeof(int));
				if(n < (int)sizeof(int))
					nDownload = 0;
				nDownload += nXfer;
				lseek(fd, SEEK_SET, 0L);
				write(fd, (char *) &nDownload, sizeof(int));
				close(fd);
			}

			art.m_pctxRoot = new MD5_CTX;
		}

		if(art.m_pctxRoot)
		{
			const char * pDBName = DBNAME;
			pPasswd = gethashpwd(pUser, pDBName);
			MD5root(art.m_pctxRoot, pUser, pPasswd, pHostAddr);
		}
#endif

		// Scan headers
		BOOL bMIME = FALSE;
		ZString stgDate, stgFrom, stgNewsgroups, stgReferences;
		ZString stgContType, stgContXfer, stgContDisp, stgMessageID;
		while((p = artspool.GetLine()))
		{
			if(*p == 0 || *p == '\r')
				break;
			
			if(strncasecmp(p, "date:", 5) == 0)
			{
				p += 5;
				while(*p && (*p == ' ' || *p == '\t')) p++;
				stgDate = p;
			}
			else if(strncasecmp(p, "from:", 5) == 0)
			{
				p += 5;
				while(*p && (*p == ' ' || *p == '\t')) p++;
				stgFrom = p;
			}
			else if(strncasecmp(p, "newsgroups:", 11) == 0)
			{
				p += 11;
				while(*p && (*p == ' ' || *p == '\t')) p++;
				stgNewsgroups = p;
			}
			else if(strncasecmp(p, "subject:", 8) == 0)
			{
				p += 8;
				while(*p && (*p == ' ' || *p == '\t')) p++;
				art.m_stgSubject = p;
			}
			else if(strncasecmp(p, "Message-ID:", 11) == 0)
			{
				p += 11;
				while(*p && (*p == ' ' || *p == '\t')) p++;
				stgMessageID = p;
			}
			else if(strncasecmp(p, "References:", 11) == 0)
			{
				p += 11;
				while(*p && (*p == ' ' || *p == '\t')) p++;
				stgReferences = p;
			}
			else if(strncasecmp(p, "mime-version:", 13) == 0)
			{
				bMIME = TRUE;
			}
			else if(bMIME)
			{
				if(strncasecmp(p, "content-type:", 13) == 0)
				{
					art.m_NewsMIME.SetMIME(TRUE);
					p += 13;
					while(*p && (*p == ' ' || *p == '\t')) p++;
					stgContType = p;
				}
				else if(strncasecmp(p, "content-transfer-encoding:", 26) == 0)
				{
					art.m_NewsMIME.SetMIME(TRUE);
					p += 26;
					while(*p && (*p == ' ' || *p == '\t')) p++;
					stgContXfer = p;
				}
				else if(strncasecmp(p, "content-disposition:", 20) == 0)
				{
					art.m_NewsMIME.SetMIME(TRUE);
					p += 20;
					while(*p && (*p == ' ' || *p == '\t')) p++;
					stgContDisp = p;
				}
			}
		}

		art.OutToolbar(stm, stgMessageID.chars());

		art.m_nHeaderCount = 1;

		//
		// Parse Article Body
		//
		if(art.m_NewsMIME.IsMIME())
		{
			if (!art.m_NewsMIME.Parse(stm, artspool.File()))
			{
				// Can't parse the article
				stm << "Article format is invalid" << endl;
				art.OutEnd(stm, tmplFile);
				return stm;
			}
		}

		stm << "<font size=-1 face=\"Arial\">" << endl;

		//
		// Display headers
		//
		if(art.m_stgSubject.length() > 0)
		{
			stm << "<TITLE>" << art.m_stgSubject << "</TITLE>" << endl;
			stm << "<b>Title: " << HTMLText(art.m_stgSubject) << "</b><br>" << endl;
		}
		else
		{
			stm << "Empty Article" << endl;
			art.OutEnd(stm, tmplFile);
			return stm;
		}

		if(stgFrom.length() > 0)
			art.pr_from(stm, stgFrom);

		if(stgDate.length() > 0)
			stm << "<b>Date: </b>" << HTMLText(stgDate) << "<br>" << endl;

/*
		if(stgNewsgroups.length() > 0)
			art.pr_newsgroups(stm, stgNewsgroups);

		if(stgReferences.length() > 0)
			art.pr_references(stm, stgReferences);

		if(stgContType.length() > 0)
			stm << "Content-Type: " << HTMLText(stgContType) << "<br>" << endl;

		if(stgContXfer.length() > 0)
			stm << "Content-Transfer-Encoding: " << HTMLText(stgContXfer)
				<< "<br>" << endl;
*/

		stm << "</font><p>" << endl;

		//
		// Display Article Body
		//
		if(art.m_NewsMIME.IsMIME())
		{
			art.m_NewsMIME.FirstPart(TRUE);
			art.pr_MIME_body(stm);
		}
		else
		{
			artspool.NoFile();
			art.pr_RFC_body(stm, artspool);
		}
	}
	else
	{
		art.OutToolbar(stm, NULL);
	}

	art.OutEnd(stm, tmplFile);
	return stm;
}

void NewsArticle::OutToolbar(ostream& stm, const char * pMessageID)
{
	stm << "<script language=javascript>" << endl
		<< "<!--" << endl;

	if(pMessageID)
		stm << "var MessageID = \"" << pMessageID << "\";" << endl;

	stm << "function NewsPost()" << endl
		<< "{" << endl
		<< "    location.href = \"" << NEWSBIN << "/wwwpost?"
		<< URLText(m_stgArticle) << "\";" << endl
		<< "}" << endl
		<< "function NewsHeader()" << endl
		<< "{" << endl
		<< "    location.href = \"" << NEWSBIN << "/wwwheader?"
		<< URLText(m_stgArticle) << "\";" << endl
		<< "}" << endl
		<< "function DrnHelp()" << endl
		<< "{" << endl
		<< "    location.href = \"" << DRN_HELP << "\";" << endl
		<< "}" << endl
		<< "function PrevArt()" << endl
		<< "{" << endl
		<< "    location.href = \"" << NEWSBIN << "/wwwprev?"
		<< URLText(m_stgArticle) << "\";" << endl
		<< "}" << endl
		<< "function NextArt()" << endl
		<< "{" << endl
		<< "    location.href = \"" << NEWSBIN << "/wwwnext?"
		<< URLText(m_stgArticle) << "\";" << endl
		<< "}" << endl
		<< "// -->" << endl
		<< "</script>" << endl
	;
}

void NewsArticle::OutEnd (ostream& stm, Zifstream& tmplFile)
{
	stm << "<p>" << DRN_NOTICE << "</p>" << endl;

	char * p;
	while(tmplFile.NextLine())
	{
		p = tmplFile.GetLine();
		stm << p << endl;
	}
}

void NewsArticle::pr_from (ostream& stm, ZString & stgFrom)
{
	ZString MailAddr;
	INMailName(stgFrom).EMailAddress(MailAddr);
	if( MailAddr.contains('@') && MailAddr.contains('.') )
		stm << "<b>Author: </b><a href=\"mailto:" << URLText(MailAddr) << "\">"
			<< HTMLText(stgFrom) << "</a><br>" << endl;
	else
		stm << "<b>Author: </b>" << HTMLText(stgFrom) << "<br>" << endl;
}

void NewsArticle::pr_newsgroups (ostream& stm, ZString & stgNewsgroups)
{
	stm << "Newsgroups: ";
	char * p = (char *)stgNewsgroups.chars();
	int n;
	char c;
	for(int i = 1 ;; i++)
	{
		n = strcspn(p, ",");
		c = p[n];
		if(c) p[n] = 0;
		if(i > 1)
			stm << ", ";
		stm << "<a href=\"" << NEWSBIN << "/wwwnews?" << URLText(p)
			<< "\">" << HTMLText(p) << "</a>";
		if(c == 0)
			break;
		p[n] = c;
		p += n+1;
	}
	stm << "<br>" << endl;
}

void NewsArticle::pr_references (ostream& stm, ZString & stgReferences)
{
	stm << "References: " << HTMLText(stgReferences) << "<br>" << endl;
/*
	stm << "References: ";
	char * p = stgReferences;
	char * q;
	int n;
	char c;
	for(int i = 1 ;; i++)
	{
		n = strcspn(p, ",");
		c = p[n];
		if(c) p[n] = 0;
		if(i > 1)
			stm << ", ";
		q = p;
		if(q[n-1] == '>')
			q[n-1] = 0;
		if(*q == '<')
			q++;
			
		stm << "<a href=\"" << NEWSBIN << "/wwwnews?" << URLText(q)
			<< "\">" << i << "</a>";
		if(c == 0)
			break;
		p += n+1;
	}
	stm << "<br>" << endl;
*/
}

void NewsArticle::pr_contdisp (ostream& stm, ZString& stgContDisp)
{
	MIMEBodyPart* pCurrentPart = m_NewsMIME.CurrentPart();
	if( pCurrentPart->Encoding() && 
		strcasecmp(pCurrentPart->Encoding(), "7bit") && 
		strcasecmp(pCurrentPart->Encoding(), "8bit") && 
		strcasecmp(pCurrentPart->Encoding(), "binary") )
	{
		stm << "Content-Disposition: " << HTMLText(pCurrentPart->Disposition());
		if (strlen(pCurrentPart->DispFileName()) > 0)
		{
			stm << "; filename=";
			char * pDigest = NULL;
#ifdef DIGESTPWD
			if(m_pctxRoot)
			{
				m_ctxCmd = *m_pctxRoot;
				unsigned char md[MDBUFLEN];
				MD5Update(&m_ctxCmd, (unsigned char *)NEWSBIN,
					strlen(NEWSBIN));
				MD5Update(&m_ctxCmd,
					(unsigned char *)"/wwwdecode?", 11);
				MD5Update(&m_ctxCmd,
					(unsigned char *)m_stgArticle.chars(),
					m_stgArticle.length());
				MD5Update(&m_ctxCmd, (unsigned char *)":", 1);
				char buf[128];
				sprintf(buf, "%d", pCurrentPart->PartNo());
				MD5cmd(&m_ctxCmd, md, buf);
				md_digest(md, m_Digest,
					gUserInfo.GetUserName());
				pDigest = m_Digest;
			}
#endif
			stm << "<a href=\"" << DECODE_SERVER;
			if(pDigest)
				stm << "/.." << pDigest;
			
			stm << NEWSBIN << "/wwwdecode?" << URLText(m_stgArticle)
				<< ":" << pCurrentPart->PartNo() << "\">" 
				<< HTMLText(pCurrentPart->DispFileName())
				<< "</a><br>" << endl;
			return;
		}
	}
	stm << "Content-Disposition: " << HTMLText(stgContDisp)
			<< "<br>" << endl;
}

void NewsArticle::pr_filename (ostream& stm)
{
	MIMEBodyPart* pCurrentPart = m_NewsMIME.CurrentPart();
	if( pCurrentPart->Encoding() && 
		( !strcasecmp(pCurrentPart->Encoding(), "base64") || 
		( !strcasecmp(pCurrentPart->Type(), "message") && 
		!strcasecmp(pCurrentPart->SubType(), "partial") )) && 
		pCurrentPart->DispFileName() && (strlen(pCurrentPart->DispFileName()) > 0) )
	{
		char * pDigest = NULL;
#ifdef DIGESTPWD
		if(m_pctxRoot)
		{
			m_ctxCmd = *m_pctxRoot;
			unsigned char md[MDBUFLEN];
			MD5Update(&m_ctxCmd, (unsigned char *)NEWSBIN,
				strlen(NEWSBIN));
			MD5Update(&m_ctxCmd,
				(unsigned char *)"/wwwdecode?", 11);
			MD5Update(&m_ctxCmd,
				(unsigned char *)m_stgArticle.chars(),
				m_stgArticle.length());
			MD5Update(&m_ctxCmd, (unsigned char *)":", 1);
			char buf[128];
			sprintf(buf, "%d", pCurrentPart->PartNo());
			MD5cmd(&m_ctxCmd, md, buf);
			md_digest(md, m_Digest, gUserInfo.GetUserName());
			pDigest = m_Digest;
		}
#endif
		stm << "<a href=\"" << DECODE_SERVER;
		if(pDigest)
			stm << "/.." << pDigest;
		stm << NEWSBIN << "/wwwdecode?" << URLText(m_stgArticle) << ":"
			<< pCurrentPart->PartNo() << "\">" 
			<< HTMLText(pCurrentPart->DispFileName())
			<< "</a><br>" << endl;
	}
}

void NewsArticle::pr_RFC_body (ostream& stm, ArticleSpool & artspool)
{
	char * p, * q;
	stm << "<img src=/drn/images/drnline.gif><pre>" << endl;
	int n;
	while((p = artspool.GetLine()))
	{
		if(strncasecmp(p, "begin", 5) == 0)
		{
			q = p+5;
			while(*q && (*q == ' ' || *q == '\t')) q++;	// strip leading spaces
			if(q[0] >= '0' && q[0] <= '7' && q[1] >= '0' && q[1] <= '7' &&
				q[2] >= '0' && q[2] <= '7')
			{
				n = 3;
				if(q[0] == '0' && (q[3] >= '0' && q[3] <= '7')) 
					n = 4;
				q[n] = 0;
				stm << "begin " << q << " ";
				q += n+1;
				while(*q && (*q == ' ' || *q == '\t')) q++;	// strip leading sp
				for( char* z=q; *z; z++ )	if (isspace(*z))	*z = '_';
				p = (char *)m_stgArticle.chars();
				
				char * pDigest = NULL;
#ifdef DIGESTPWD
				if(m_pctxRoot)
				{
					m_ctxCmd = *m_pctxRoot;
					unsigned char md[MDBUFLEN];
					MD5Update(&m_ctxCmd,
						(unsigned char *)NEWSBIN,
						strlen(NEWSBIN));
					MD5Update(&m_ctxCmd,
						(unsigned char *)"/wwwdecode?",
						11);
					MD5cmd(&m_ctxCmd, md,
						m_stgArticle.chars());
					md_digest(md, m_Digest,
						gUserInfo.GetUserName());
					pDigest = m_Digest;
				}
#endif
				stm << "<a href=\"" << DECODE_SERVER;
				if(pDigest)
					stm << "/.." << pDigest;
				stm << NEWSBIN << "/wwwdecode?"
					<< URLText(m_stgArticle) << "\">"
					<< HTMLText(q) << "</a>" << endl;
				continue;
			}
		}

		pr_line(stm, p);
	}
	stm << "</pre>" << endl;
}

void NewsArticle::pr_MIME_headers (ostream& stm)
{
	ZString stgContType, stgContXfer, stgContDisp;
	MIMEBodyPart* pCurrentPart = m_NewsMIME.CurrentPart();
	MIMEParamList* pParamList = pCurrentPart->ParamList();
	MIMEParamTuple* pTuple = NULL;
	stm << "<img src=/drn/images/drnline.gif><font size=-1><br>" << endl;

	stgContType = (ZString)pCurrentPart->Type() + "/" + 
		pCurrentPart->SubType();
	pTuple = pParamList->GetParam("charset");
	if (pTuple)
		stgContType = stgContType + "; " + pTuple->Param() + 
			"=" + pTuple->Value();
	if(stgContType.length() > 0)
		stm << "Content-Type: " << HTMLText(stgContType) 
			<< "<br>" << endl;

	stgContXfer = (ZString)pCurrentPart->Encoding();
	if(stgContXfer.length() > 0)
		stm << "Content-Transfer-Encoding: " << HTMLText(stgContXfer) 
			<< "<br>" << endl;

	stm << "</font><p>" << endl;
}

void NewsArticle::pr_MIME_body (ostream& stm)
{
	int nLen;
	ZString szBuffer;
	BOOL bHTML;
	for( MIMEBodyPart* pCurrentPart = m_NewsMIME.CurrentPart(); 
		pCurrentPart; 
		pCurrentPart = pCurrentPart->Sibling() )
	{
		m_NewsMIME.SetCurrent(pCurrentPart);
		pCurrentPart->Open();
		bHTML = pCurrentPart->IsHTML();
		for( nLen = pCurrentPart->FirstLine(szBuffer, stm); 
			nLen > 0; 
			nLen = pCurrentPart->NextLine(szBuffer, stm) )
		{}
		if( !strcasecmp(pCurrentPart->Type(), "multipart") && 
			pCurrentPart->FirstChild() )
		{
			char* pBoundary = NULL;
			int n = 0;
			nLen = pCurrentPart->NextLine(szBuffer, stm);
			MIMEParamList* pParamList = pCurrentPart->ParamList();
			MIMEParamTuple* pTuple = NULL;
			pTuple = pParamList->GetParam("boundary");
			if (pTuple)
			{
				pBoundary = pTuple->Value();
				n = strlen(pBoundary);
			}
			if( (nLen > -1) && 
				( (nLen < n+2) || (szBuffer[0] != '-') ||
				(szBuffer[1] != '-') || 
				strncmp(&szBuffer[2], pBoundary, n) ))
			{
				if (pCurrentPart->PartNo() > 0)
					pr_MIME_headers(stm);
				else
					// Draw a line between different body parts
					stm << "<img src=/drn/images/drnline.gif>" << endl;
				pr_filename(stm);
				stm << (bHTML?"<p>":"<pre>") << endl;
				for( ; nLen > -1; 
					nLen = pCurrentPart->NextLine(szBuffer, stm) )
					if( (nLen < n+2) || (szBuffer[0] != '-') ||
						(szBuffer[1] != '-') || 
						strncmp(&szBuffer[2], pBoundary, n) )
						pr_strline(stm, szBuffer, bHTML);
					else
						break;
				stm << (bHTML?"<p>":"</pre>") << endl;
			}
			m_NewsMIME.SetCurrent(pCurrentPart->FirstChild());
			pr_MIME_body(stm);
			m_NewsMIME.SetCurrent(pCurrentPart);
			BOOL bEnd = FALSE;
			for( nLen = pCurrentPart->NextLine(szBuffer, stm); 
				nLen > -1; 
				nLen = pCurrentPart->NextLine(szBuffer, stm) )
			{
				if (bEnd)
					pr_strline(stm, szBuffer, bHTML);
				else if( (nLen > n+3) && (szBuffer[0] == '-') && 
					(szBuffer[1] == '-') &&
					!strncmp(&szBuffer[2], pBoundary, n) && 
					(szBuffer[n+2] == '-') && (szBuffer[n+3] == '-') )
				{
					stm << "<img src=\"/drn/images/drnline.gif\"><br>" << endl;
					bEnd = TRUE;
				}
			}
		}
		else
		{
			if (pCurrentPart->PartNo() > 0)
				pr_MIME_headers(stm);
			else
				// Draw a line between different body parts
				stm << "<img src=\"/drn/images/drnline.gif\"><br>" << endl;
			pr_filename(stm);
			stm << (bHTML?"<p>":"<pre>") << endl;
			for( nLen = pCurrentPart->NextLine(szBuffer, stm); 
				nLen > -1; 
				nLen = pCurrentPart->NextLine(szBuffer, stm) )
				pr_strline(stm, szBuffer, bHTML);
			if(bHTML)
				stm << "<p><base href=" << DRN_SERVER << NEWSBIN << "/>"
					<< endl;
			else
				stm << "</pre>" << endl;
		}
		pCurrentPart->Close();
	}
}

void NewsArticle::pr_strline (ostream& stm, ZString& szBuffer, BOOL bHTML)
{
	MIMEBodyPart* pCurrentPart = m_NewsMIME.CurrentPart();
	if( pCurrentPart->Encoding() &&
		!strcasecmp(pCurrentPart->Encoding(), "base64") )
		stm << szBuffer << (bHTML?"<br>":"") << endl;
	else
	{
		char * p = (char *)szBuffer.chars();
		if(strncasecmp(p, "begin", 5) == 0)
		{
			char* q = p+5;
			while(*q && (*q == ' ' || *q == '\t')) q++;	// strip leading spaces
			if(q[0] >= '0' && q[0] <= '7' && q[1] >= '0' && q[1] <= '7' &&
				q[2] >= '0' && q[2] <= '7')
			{
				q[3] = 0;
				stm << "begin " << q << " ";
				q += 4;
				while(*q && (*q == ' ' || *q == '\t')) q++;	// strip leading sp
				for( char* z=q; *z; z++ )	if (isspace(*z))	*z = '_';
				p = (char *)m_stgArticle.chars();

				char * pDigest = NULL;
#ifdef DIGESTPWD
				if(m_pctxRoot)
				{
					m_ctxCmd = *m_pctxRoot;
					unsigned char md[MDBUFLEN];
					MD5Update(&m_ctxCmd,
						(unsigned char *)NEWSBIN,
						strlen(NEWSBIN));
					MD5Update(&m_ctxCmd,
						(unsigned char *)"/wwwdecode?",
						11);
					MD5cmd(&m_ctxCmd, md,
						m_stgArticle.chars());
					md_digest(md, m_Digest,
						gUserInfo.GetUserName());
					pDigest = m_Digest;
				}
#endif
				stm << "<a href=\"" << DECODE_SERVER;
				if(pDigest)
					stm << "/.." << pDigest;
				stm << NEWSBIN << "/wwwdecode?"
					<< URLText(m_stgArticle) << "\">"
					<< HTMLText(q) << "</a>"
					<< (bHTML?"<br>":"") << endl;
				return;
			}
		}
		if(bHTML)
			stm << p << endl;
		else
			pr_line(stm, p);
	}
}

void NewsArticle::pr_line (ostream& stm, char * p)
{
	int n;
	for(char * q = p; *p ; p+=n)
	{
		n = strcspn(p, "&<:");
		switch(p[n])
		{
		case '&':
			stm.write(q, n);
			stm << "&amp;";
			n++;
			q = p + n;
			continue;
		case '<':
			stm.write(q, n);
			stm << "&lt;";
			n++;
			q = p + n;
			continue;
		case ':':
			//
			// no longer scanning for mailto, news, newspost, nntp, telnet,
			//	gopher, tn3270 and whois
			//
			if(strncmp(p+n-4, "http", 4) == 0)
			{
				if(n > 4)
					stm.write(q, n-4);
				p = p + n - 4;
				n = pr_link(stm, p);
				q = p + n;
			}
			else if(strncmp(p+n-3, "ftp", 3) == 0)
			{
				if(n > 3)
					stm.write(q, n-3);
				p = p + n - 3;
				n = pr_link(stm, p);
				q = p + n;
			}
			else
			{
				stm.write(q, ++n);
				q = p+n;
			}
			continue;
		}
		stm << q;
		break;
	}
	stm << endl;
}

int NewsArticle::pr_link (ostream& stm, char *p)
{
	char c;
	int n = 0;
	while((c = p[n]))
	{
		if(c == ' ' || c == '\t' || c == '"' || c == '<' || c == '>' ||
			c == '(' || c == ')' || c == '=' || c == '[' || c == ']')
			break;
		n++;
	}
	if(p[n-1] == '.' || p[n-1] == ',')
		c = p[--n];
	if(c) p[n] = 0;
	stm << "<a href=\"" << URLText(p) << "\">" << HTMLText(p) << "</a>";
	if(c) p[n] = c;
	return n;
}
