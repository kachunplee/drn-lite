#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>

#ifdef LINUX
#include <sys/file.h>
#endif

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>

#include "def.h"
#include "userinfo.h"
#include "dgauth.h"

#include "tmplerr.h"
#include "artspool.h"

#ifdef GROUP_SUB
#include "newsacc.h"
#endif

#include "base64.h"
#include "uudecode.h"
#include "hexbin.h"
#include "newslover.h"
#include "nntpov.h"
#include "newsdecode.h"
#include "drncache.h"

#ifndef DIGESTPWD
extern char * gethashpwd(const char *, const char *);
extern char * mkdigest(char *, const char *, const char *, const char *, const char *);
#endif

const char NameTbl [] =
	"____________________________________________,-._0123456789___=__"
	"@ABCDEFGHIJKLMNOPQRSTUVWXYZ______abcdefghijklmnopqrstuvwxyz_____";

TemplateError tmplerr;
extern const int OV_DELAY;;
extern BOOL MakeOVDirectory(const char *);
const char f_artnum_i [] = "artnum.i";

NewsDecode::~NewsDecode ()
{
	delete [] m_pSubjects;
	delete [] m_pNames;
	delete [] m_pArtNums;
	delete [] m_pMessageIDs;
	delete [] m_pArtSizes;
	if (m_pFile)
		CloseWriteFile();

	if(m_pfileOver)
		m_pfileOver->close();

	UnLockFile(LockFileName, &m_nLockFile);
	UnLockFile(szMessageID, &m_nLockMsg);
}

ostream& operator << (ostream& stm, NewsDecode& decode)
{
	decode.m_stgGroup = decode.m_stgArticle;
	int nArticleNo = 0;
	const char * pGroup = decode.m_stgGroup;
	const char * qGroup = strrchr(pGroup, '/');
	if(qGroup)
	{
		const char * pArt = ++qGroup;
		for(; *qGroup; qGroup++) { if(!isdigit(*qGroup)) break; }
		if(*qGroup == 0)
		{
			nArticleNo = pArt - pGroup;
			decode.m_stgGroup[nArticleNo-1] = 0;
		}
	}
	decode.m_dirGroup = decode.m_stgGroup.chars();
	decode.m_dirGroup.gsub('.', '/');

#ifdef GROUP_SUB
	NewsAccess newsAccess;
	if(!newsAccess.IsGroupSubscribe(decode.m_stgGroup))
	{
		tmplerr.OutContentType(stm);
		tmplerr.OutError(stm, NO_GROUP_MSG);
		return stm;
	}
#endif

	ArticleSpool artspool(decode.m_stgGroup,
			decode.m_stgArticle.chars() + nArticleNo);

	ZString sErr;
	if(!artspool.good())
	{
		tmplerr.OutContentType(stm);
		sErr = "<h2>";
		sErr += artspool.Errmsg();
		sErr += "</h2>";
		tmplerr.OutError(stm, sErr.chars());
		return stm;
	}

	// Scan headers
	char * p;
	BOOL bMIME = FALSE;
	while((p = artspool.GetLine()) != NULL)
	{
		if(*p == 0 || *p == '\r')
			break;
		
		if(strncasecmp(p, "subject:", 8) == 0)
		{
			p += 8;
			while(*p && (*p == ' ' || *p == '\t')) p++;
			decode.m_Subject.stg() = p;
		}
		else if(strncasecmp(p, "message-id:", 11) == 0)
		{
			p += 11;
			while(*p && (*p == ' ' || *p == '\t')) p++;
			if(*p && *p == '<') p++;
			decode.m_MessageID = p;
			if(decode.m_MessageID.lastchar() == '>')
			{
				int n = decode.m_MessageID.length() - 1;
				decode.m_MessageID.del(n, 1);
			}
		}
		else if(strncasecmp(p, "mime-version:", 13) == 0)
		{
			bMIME = TRUE;
		}
		else if(bMIME && strncasecmp(p, "content-type:", 13) == 0)
			decode.m_NewsMIME.SetMIME(TRUE);
	}

	decode.GetNameFromSubject(decode.m_DecodeName);
	if(decode.m_DecodeName.length() > 0)
	{
		if(decode.FindCache(stm))
			return stm;
		decode.UnLockFile(LockFileName, &(decode.m_nLockFile));
	}

	//
	// Check headers
	//
	if(decode.m_Subject.stg().length() <= 0)
	{
		tmplerr.OutContentType(stm);
		tmplerr.OutError(stm, "<h2>Empty Article</h2>");
		return stm;
	}

	//
	// Article Body
	//
	if(decode.m_NewsMIME.IsMIME())
	{
		if(decode.m_NewsMIME.Parse(stm, artspool.File()))
			decode.DecodeMIME(stm);
		else
		{
			// Can't parse the article
			stm << "Content-Type: text/html" << endl << endl
				<< DEF_BODY_TAG << endl
				<< "<h2>Invalid MIME format</h2>" << endl;
			return stm;
		}
	}
	else
	{
		artspool.NoFile();
		decode.DecodeUU(stm, artspool);
	}
	return stm;
}

void NewsDecode::DecodeMIME (ostream& stm)
{
	m_bMulti = FALSE;
	if (m_nDecodePart == -2)
	{
		m_bForceDecode = TRUE;
		m_bMulti = TRUE;
		m_nDecodePart = 0;
	}
	else if (m_nDecodePart == -1)
		m_nDecodePart = 0;
	m_NewsMIME.FirstPart(TRUE);
	MIMEBodyPart* pCurrentPart = m_NewsMIME.GetBodyPart(stm, m_nDecodePart);
	if (!pCurrentPart)
	{
		// Can't find body part
		stm << "Content-Type: text/html" << endl << endl
			<< DEF_BODY_TAG << endl
			<< "<h2>Cannot find article</h2>" << endl;
		return;
	}
	int n = Decode_bodypart(stm);
	if (n == RESULT_ERR)
	{
		// Can't decode body part
		stm << "Content-Type: text/html" << endl << endl
			<< DEF_BODY_TAG << endl
			<< "<h2>Decode format not recognized</h2>" << endl;
		return;
	}
	else if (n == RESULT_ERRASSERT || n == RESULT_NONE)
	{
		stm << "Content-Type: text/html" << endl << endl
			<< DEF_BODY_TAG << endl
			<< "<h2>Cannot decode article</h2>" << endl;

		return;
	}
	else if (n != RESULT_OK)
		return;
}

int NewsDecode::Decode_bodypart (ostream& stm)
{
	MIMEBodyPart* pCurrentPart = m_NewsMIME.CurrentPart();
	if (!pCurrentPart)
		return RESULT_ERRASSERT;
	if (!strcasecmp(pCurrentPart->Type(), "message"))
	{
		if (!strcasecmp(pCurrentPart->SubType(), "partial"))
		{
			if (strcasecmp(pCurrentPart->Encoding(), "7bit"))
			{
				stm << "Content-Type: text/html" << endl << endl
					<< DEF_BODY_TAG << endl
					<< "<h2>Invalid MIME Content-Transfer-Encoding format</h2>" 
					<< endl;
				return RESULT_ERRMSG;
			}
			if (m_nMulti)
			{
				stm << "Content-Type: text/html" << endl << endl
					<< DEF_BODY_TAG << endl
					<< "<h2>Reassembled message/partial Content-Type format not supported</h2>" 
					<< endl;
				return RESULT_ERRMSG;
			}
			m_nMulti = MULTI_MIME;
			return Decode_multifile(stm);
		}
		else if( strcasecmp(pCurrentPart->Encoding(), "7bit") && 
			strcasecmp(pCurrentPart->Encoding(), "8bit") && 
			strcasecmp(pCurrentPart->Encoding(), "binary") )
		{
			stm << "Content-Type: text/html" << endl << endl
				<< DEF_BODY_TAG << endl
				<< "<h2>Invalid MIME Content-Transfer-Encoding format</h2>" 
				<< endl;
			return RESULT_ERRMSG;
		}
		return Process_encoding(stm);
	}
	else if (!strcasecmp(pCurrentPart->Type(), "multipart"))
	{
		if( strcasecmp(pCurrentPart->Encoding(), "7bit") && 
			strcasecmp(pCurrentPart->Encoding(), "8bit") && 
			strcasecmp(pCurrentPart->Encoding(), "binary") )
		{
			stm << "Content-Type: text/html" << endl << endl
				<< DEF_BODY_TAG << endl
				<< "<h2>Invalid MIME Content-Transfer-Encoding format</h2>" 
				<< endl;
			return RESULT_ERRMSG;
		}
		int n = Process_encoding(stm);
		if( (n != RESULT_OK) && (n != RESULT_ERR) && (n != RESULT_NONE) )
			return n;
		return Decode_multipart(stm);
	}
	else
	{
		return Process_encoding(stm);
	}
}

int NewsDecode::Decode_multipart (ostream& stm)
{
	MIMEBodyPart *pCurrentPart = NULL, *pParent = m_NewsMIME.CurrentPart();
	if (!pParent)
		return RESULT_ERRASSERT;
	BOOL bMixed = FALSE;
	BOOL bDisplay = FALSE;
	int n = RESULT_ERR;
	if (!strcasecmp(pParent->SubType(), "mixed"))
		bMixed = TRUE;
	else if (!strcasecmp(pParent->SubType(), "alternative"))
	{
		pCurrentPart = pParent->LastChild();
		if (!pCurrentPart)
			return RESULT_ERR;
		for( ; pCurrentPart; 
			pCurrentPart = pCurrentPart->PrevSibling() )
		{
			m_NewsMIME.SetCurrent(pCurrentPart);
			n = Decode_bodypart(stm);
			if (n == RESULT_OK)
				bDisplay = TRUE;
			if( (n != RESULT_ERR) && (n != RESULT_NONE) )
				return n;
		}
	}
	else
		bMixed = TRUE;
	if (bMixed)
	{
		pCurrentPart = pParent->FirstChild();
		if (!pCurrentPart)
			return RESULT_ERR;
		for( ; pCurrentPart; 
			pCurrentPart = pCurrentPart->Sibling() )
		{
			m_NewsMIME.SetCurrent(pCurrentPart);
			n = Decode_bodypart(stm);
			if (n == RESULT_OK)
				bDisplay = TRUE;
			if( (n != RESULT_OK) && (n != RESULT_ERR) && (n != RESULT_NONE) )
				return n;
		}
	}

	if (pParent != pCurrentPart)
		m_NewsMIME.SetCurrent(pParent);
	if (bDisplay)
		return RESULT_OK;
	return RESULT_ERR;
}

int NewsDecode::Decode_multifile (ostream& stm)
{
	MIMEBodyPart* pCurrentPart = m_NewsMIME.CurrentPart();
	if (!pCurrentPart)
		return RESULT_ERRASSERT;
	if (!m_bMulti)
	{
		int n = Get_multifile(stm);
		if (n != RESULT_OK)
			return n;
		for( int i=1; i<=m_nTotalPart; i++ )
		{
			if (m_pArtNums[i].length() == 0)
			{
				Display_missing(stm);
				return RESULT_ERRMSG;
			}
		}
	}

	if (m_nMulti == MULTI_MIME)
		return MultiMIMEDecode(stm);
	else if (m_nMulti == MULTI_UUE)
	{
		MultiUUEDecode(stm);
		return RESULT_OK;
	}
	else
		return RESULT_ERR;
}

int NewsDecode::Get_multifile (ostream& stm)
{
	MIMEBodyPart* pCurrentPart = m_NewsMIME.CurrentPart();
	if (!pCurrentPart)
		return RESULT_ERRASSERT;

	// Multiple file
	if(!OpenOver())
	{
		stm << "Content-Type: text/html" << endl << endl
			<< DEF_BODY_TAG << endl
			<< "<h2>Cannot open " << HTMLText(m_stgGroup)
			<< " Group List to complete decode of multi-file article</h2>" 
			<< endl;
		return RESULT_ERRMSG;
	}

	if (m_nMulti == MULTI_MIME)
	{
		char *pId=NULL, *pNo=NULL, *pTotal=NULL;
		MIMEParamList* pParamList = pCurrentPart->ParamList();
		if (!pParamList)
			return RESULT_ERRASSERT;
		MIMEParamTuple* pTuple = NULL;
		pTuple = pParamList->GetParam("id");
		if (pTuple)
			pId = pTuple->Value();
		if (!pId)
		{
			stm << "Content-Type: text/html" << endl << endl
				<< DEF_BODY_TAG << endl
				<< "<h2>Missing id parameter.. Invalid MIME Content-Type format</h2>" 
				<< endl;
			return RESULT_ERRMSG;
		}
		pTuple = pParamList->GetParam("number");
		if (pTuple)
			pNo = pTuple->Value();
		if (!pNo)
		{
			stm << "Content-Type: text/html" << endl << endl
				<< DEF_BODY_TAG << endl
				<< "<h2>Missing number parameter.. Invalid MIME Content-Type format</h2>" 
				<< endl;
			return RESULT_ERRMSG;
		}
		m_nTotalPart = atoi(pNo);
		int nSize = m_nTotalPart+1;
		pTuple = pParamList->GetParam("total");
		if (pTuple)
			pTotal = pTuple->Value();
		if( pTotal && (atoi(pTotal)+1 > nSize) )
		{
			m_nTotalPart = atoi(pTotal);
			nSize = m_nTotalPart+1;
		}
		m_pSubjects = new ZString[nSize];
		m_pNames = new ZString[nSize];
		m_pArtNums = new ZString[nSize];
		m_pMessageIDs = new ZString[nSize];
		m_pArtSizes = new int[nSize];

		int i, nPart, nCount=0;
		char *p=NULL, *q=NULL, *pszArtNum=NULL;
		pParamList = NULL;
		pTuple = NULL;
		do
		{
			p = m_pfileOver->GetLine();
			if (*p == '\0' || !atoi(p))
				continue;
			pszArtNum = p;
			while( *p && (*p != '\t') ) p++;
			*p++ = 0;
			ArticleSpool artspool(m_stgGroup, pszArtNum);
			if( !artspool.good() )
				return RESULT_ERRASSERT;
			ZString id, no, total, messageid;
			char *pl, *ql;
			while((pl = artspool.GetLine()))
			{
				BOOL bColon = FALSE;
				if( (*pl == 0) || (*pl == '\r') )
					break;
				if (!strncasecmp(pl, "content-type:", 13))
				{
					pl += 13;
					while( *pl && (*pl == ' ' || *pl == '\t') ) pl++;
					if (!strncasecmp(pl, "message/partial", 15))
					{
						pl += 15;
						while( *pl && (*pl == ' ' || *pl == '\t') ) pl++;
						while (*pl)
						{
							bColon = FALSE;
							if (!strncasecmp(pl, "id", 2))
							{
								pl += 2;
								while( *pl && (*pl == ' ' || *pl == '\t' || 
									*pl == '=' || *pl == '"') ) pl++;
								ql = pl;
								while( *pl && (*pl != '\r') && (*pl != '\n') && 
									(*pl != '"') && (*pl != ';') )
									pl++;
								if (*pl == ';')
									bColon = TRUE;
								*pl = 0;
								id = ql;
								if (strcmp(pId, id))
									break;
							}
							else if (!strncasecmp(pl, "number", 6))
							{
								pl += 6;
								while( *pl && (*pl == ' ' || *pl == '\t' || 
									*pl == '=' || *pl == '"') ) pl++;
								ql = pl;
								while( *pl && (*pl != '\r') && (*pl != '\n') && 
									(*pl != '"') && (*pl != ';') )
									pl++;
								if (*pl == ';')
									bColon = TRUE;
								*pl = 0;
								no = ql;
							}
							else if (!strncasecmp(pl, "total", 2))
							{
								pl += 5;
								while( *pl && (*pl == ' ' || *pl == '\t' || 
									*pl == '=' || *pl == '"') ) pl++;
								ql = pl;
								while( *pl && (*pl != '\r') && (*pl != '\n') && 
									(*pl != '"') && (*pl != ';') )
									pl++;
								if (*pl == ';')
									bColon = TRUE;
								*pl = 0;
								total = ql;
							}
							if( (*pl == ';' || bColon) && !*(++pl) )
							{
								pl = artspool.GetLine();
								if((pl == NULL) || (*pl == 0) || (*pl == '\r'))
									break;
							}
							else
								pl++;
						}
					}
				}
				else if (!strncasecmp(pl, "message-id:", 11))
				{
					pl += 11;
					while( *pl && (*pl == ' ' || *pl == '\t') ) pl++;
					if( *pl && *pl == '<') pl++;
					messageid = pl;
					if (messageid.lastchar() == '>')
					{
						int n = messageid.length() - 1;
						messageid.del(n, 1);
					}
				}
			} while(m_pfileOver->NextLine());
			if( id.length() < 1 || no.length() < 1 )
				continue;

			nPart = atoi(no);
			if (nPart > m_nTotalPart)
				Adjust_multifilearray(nPart+1);
			if( (m_pArtNums[nPart].length()<1) && nPart )
				nCount++;
			m_pMessageIDs[nPart] = messageid;
			m_pArtNums[nPart] = pszArtNum;
			q = p;
			while( *p && (*p != '\t') )	p++;
			*p++ = 0;
			NewsSubject Subject = q;
			m_pSubjects[nPart] = Subject.stg();
			q = p;
			while( *p && (*p != '\t') )	p++;
			*p++ = 0;
			INMailName(q).RealName(m_pNames[nPart]);
			for( i=3; i<7; i++ )
			{
				while(*p && *p != '\t') p++;
				p++;
			}
			m_pArtSizes[nPart] = atoi(p);
			if (total.length())
				nSize = atoi(total)+1;
			if (nSize > m_nTotalPart+1)
				Adjust_multifilearray(nSize);

			if (nCount == m_nTotalPart)
				break;
		} while(m_pfileOver->NextLine());
	}
	else if (m_nMulti == MULTI_UUE)
	{
		int nSize = m_nTotalPart+1;
		m_pSubjects = new ZString[nSize];
		m_pNames = new ZString[nSize];
		m_pArtNums = new ZString[nSize];
		m_pMessageIDs = new ZString[nSize];
		m_pArtSizes = new int[nSize];

		ZString stgPattern;
		m_Subject.IsMultiPart(stgPattern);
#ifdef TRACE
		cerr << "MULTI_UUE " << stgPattern << endl;
#endif

		int i, nPart, nCount=0;
		ZString	tmpString;
		char *p=NULL, *pszArtNum=NULL;
		do
		{
			p = m_pfileOver->GetLine();
			if (*p == '\0' || !atoi(p))
				continue;
			pszArtNum = p;							// 1st = Article Number
			while( *p && (*p != '\t') )	p++;
			*p++ = 0;

			char* q = p;							// 2nd = Subject
			while( *p && (*p != '\t') )	p++;
			*p++ = 0;
			NewsSubject Subject = q;

			tmpString = q;
			if( Subject.IsMultiPart(tmpString, NULL, &nPart) && 
				(nPart > 0) &&
				(nPart <= m_nTotalPart) && 
#ifdef TRACE
				(cerr << tmpString << endl, tmpString == stgPattern) )
#else
				(tmpString == stgPattern) )
#endif
			{
				if( (m_pArtNums[nPart].length()<1) && nPart )
					nCount++;
				m_pSubjects[nPart] = Subject.stg();
				m_pArtNums[nPart] = pszArtNum;

				q = p;								// 3rd = Author
				while(*p && *p != '\t') p++;
				*p++ = 0;
				INMailName(q).RealName(m_pNames[nPart]);

				for(i = 3; i < 7; i++)				// 7th = no. of line
				{
					while(*p && *p != '\t') p++;
					p++;
				}
				m_pArtSizes[nPart] = atoi(p);
			}

			if (nCount == m_nTotalPart)
				break;
		} while(m_pfileOver->NextLine());
	}
	else
		return RESULT_ERR;
	return RESULT_OK;
}

void NewsDecode::Adjust_multifilearray (int nSize)
{
	int i;
	ZString* pszTemp = m_pSubjects;
	m_pSubjects = new ZString[nSize];
	for( i=0; i<m_nTotalPart+1; i++ )
		m_pSubjects[i] = pszTemp[i];
	delete [] pszTemp;
	pszTemp = m_pNames;
	m_pNames = new ZString[nSize];
	for( i=0; i<m_nTotalPart+1; i++ )
		m_pNames[i] = pszTemp[i];
	delete [] pszTemp;
	pszTemp = m_pArtNums;
	m_pArtNums = new ZString[nSize];
	for( i=0; i<m_nTotalPart+1; i++ )
		m_pArtNums[i] = pszTemp[i];
	delete [] pszTemp;
	pszTemp = m_pMessageIDs;
	m_pMessageIDs = new ZString[nSize];
	for( i=0; i<m_nTotalPart+1; i++ )
		m_pMessageIDs[i] = pszTemp[i];
	delete [] pszTemp;
	int* pnTemp = m_pArtSizes;
	m_pArtSizes = new int[nSize];
	for( i=0; i<m_nTotalPart+1; i++ )
		m_pArtSizes[i] = pnTemp[i];
	delete [] pnTemp;
	m_nTotalPart = nSize - 1;
}

BOOL NewsDecode::InitMultiFileDecode (PCGI pCGI)
{
	m_stgGroup = cgi_GetValueByName(pCGI, "stgNewsName");
	if (m_stgGroup.length() < 1)
		return FALSE;
	m_nTotalPart = atoi(cgi_GetValueByName(pCGI, "nTotalPart"));
	if (m_nTotalPart < 2)
		return FALSE;
	m_pArtNums = new ZString[m_nTotalPart+1];
	if (!m_pArtNums)
		return FALSE;
	int i, j;
	char* p;
	for( i=0; (i < m_nTotalPart+1); i++ )
	{
		if (!(p = cgi_GetName(pCGI, i+4)))
			break;
		if( ((j = atoi(p)) < m_nTotalPart) || (j > -1) )
		{
			m_pArtNums[j] = cgi_GetValue(pCGI, i+4);
		}
	}
	m_nMulti = atoi(cgi_GetValueByName(pCGI, "nMulti"));
	if(m_nMulti)
		return TRUE;
	return FALSE;
}

void NewsDecode::Display_missing (ostream& stm)
{
	stm << "Content-Type: text/html" << endl << endl
		<< DEF_BODY_TAG << endl
		<< "<title>Missing parts</title>" << endl
		<< "<h2>Multiple part article missing parts</h2><hr>" << endl
		<< "<form method=POST action=\"" << NEWSBIN 
		<< "/wwwmdecode\">" << endl
		<< "<input type=hidden name=\"stgNewsName\" value=\"" 
		<< URLText(m_stgGroup) << "\">" << endl
		<< "<input type=hidden name=\"stgArticle\" value=\"" 
		<< URLText(m_stgArticle) << "\">" << endl
		<< "<input type=hidden name=\"nTotalPart\" value=\"" 
		<< m_nTotalPart << "\">" << endl
		<< "<input type=hidden name=\"nMulti\" value=\"" 
		<< m_nMulti << "\">" << endl
		<< "<pre>" << endl;

	ZString stgName = m_stgGroup;;
	int nMissing=0;
	for( int i=0; i<=m_nTotalPart; i++ )
	{
		if (m_pArtNums[i].length() < 1)
		{
			if (!nMissing)
				nMissing = i;
			continue;
		}
		if(nMissing > 0)
		{
			stm << "<br><strong>Missing parts " << nMissing;
			if(nMissing != (i-1))
			{
				nMissing = i - 1;
				stm << "-" << nMissing;
			}
			stm << "</strong>" << endl;
			nMissing = 0;
		}
		stm << "<input type=\"hidden\" name=\"" << i << "\" value=\"" 
			<< m_pArtNums[i] << "\">"
			<< "<IMG SRC=\"/drn/images/image.gif\">";
		stm.form(" %20.20s %5d ", m_pNames[i].chars(), m_pArtSizes[i]);
		stm	<< "<a href=\"" << ARTICLE_SERVER << NEWSBIN << "/wwwnews?"
			<< URLText(stgName) << "/" << m_pArtNums[i] << "\">" 
			<< HTMLText(m_pSubjects[i]) << "</a>" << endl;
	}
	if (nMissing > 0)
	{
		stm << "<strong>Missing parts " << nMissing;
		if (nMissing != m_nTotalPart)
			stm << "-" << m_nTotalPart;
		stm << "</strong>" << endl;
	}
	stm << "<p><input type=submit value=\"Try Decode Them Anyway\">" 
		<< "</form>" << endl;
}

int NewsDecode::Process_encoding (ostream& stm)
{
	MIMEBodyPart* pCurrentPart = m_NewsMIME.CurrentPart();
	if (!pCurrentPart)
		return RESULT_ERRASSERT;
	if (!pCurrentPart->Open())
		return RESULT_ERRASSERT;

	//
	// Skip the headers
	//
	int nLen;
	ZString szBuffer;
	for( nLen = pCurrentPart->FirstLine(szBuffer, stm); 
		nLen > 0; 
		nLen = pCurrentPart->NextLine(szBuffer, stm) )
	{}

	char* pBoundary = NULL;
	BOOL bMultipart = FALSE;
	if( !strcasecmp(pCurrentPart->Type(), "multipart") && 
		pCurrentPart->FirstChild() )
	{
		bMultipart = TRUE;
		MIMEParamList* pParamList = pCurrentPart->ParamList();
		MIMEParamTuple* pTuple = pParamList->GetParam("boundary");
		if (pTuple)
			pBoundary = pTuple->Value();
	}

	if (!strcasecmp(pCurrentPart->Encoding(), "base64"))
	{
		if (!pCurrentPart->DispFileName())
		{
			stm << "Content-Type: text/html" << endl << endl
				<< DEF_BODY_TAG << endl
				<< "<h2>Cannot decode article</h2>" << endl;
			return RESULT_ERRMSG;
		}
		m_DecodeName = pCurrentPart->DispFileName();
		for(unsigned int i = 0; i < m_DecodeName.length() ; i++)
		{
			// Convert all illegal chars in filename to underscore
			if(m_DecodeName[i] >= 0)
				m_DecodeName[i] = NameTbl[m_DecodeName[i]];
		}

		if(FindCache(stm))
			return RESULT_OK;

		CreateCacheName(stm, m_DecodeName);

		Base64Decode b64;
		if (!b64.Open(m_stgHashName))
			return RESULT_ERRASSERT;
		b64.SetMode();

		if( !strcasecmp(pCurrentPart->Type(), "multipart") && 
			pCurrentPart->FirstChild() )
		{
			nLen = pCurrentPart->NextLine(szBuffer, stm);
			if( (nLen > -1) && 
				( (szBuffer[0] != '-') || (szBuffer[1] != '-') || 
				strncmp(&szBuffer[2], pBoundary, strlen(pBoundary)) ))
			{
				for( ; nLen > -1; 
					nLen = pCurrentPart->NextLine(szBuffer, stm) )
					if( (szBuffer[0] != '-') || (szBuffer[1] != '-') || 
						strncmp(&szBuffer[2], pBoundary, strlen(pBoundary)) )
				    	b64.Decode(szBuffer);
					else
						break;
			}
		}
		else
		{
			for( nLen = pCurrentPart->NextLine(szBuffer, stm); 
				nLen > -1; 
				nLen = pCurrentPart->NextLine(szBuffer, stm) )
			    b64.Decode(szBuffer);
		}

		pCurrentPart->Close();
		b64.Close();
	}
	else
	{
		if (m_nMulti == MULTI_UUE)
		{
			MultiUUEDecode(stm);
			return RESULT_OK;
		}

		if( !m_nMulti && 
			m_Subject.IsMultiPart(&m_nPartNo, &m_nTotalPart) && 
			(m_nTotalPart > 1) && m_nPartNo > 0 )
		{
			m_nMulti = MULTI_UUE;
			return Decode_multifile(stm);
		}
		UUDecode * pUUdecode = NULL;
		for(nLen = pCurrentPart->NextLine(szBuffer, stm);
			nLen > -1;
			nLen = pCurrentPart->NextLine(szBuffer, stm))
		{
			if(bMultipart && nLen == 0)
				break;

			if(FindUUBegin(stm, szBuffer))
			{
				if(FindCache(stm))
					return RESULT_OK;

				CreateCacheName(stm, m_DecodeName);

				// Start UU decode
				pUUdecode = new UUDecode;
				if(!pUUdecode->Open(m_stgHashName))
				{
					stm << "Content-Type: text/html" << endl << endl
						<< DEF_BODY_TAG << endl
						<< "<h2>Cannot create the decoded file "
						<< HTMLText(m_stgHashName)
						<< "</h2>" << endl;
					return RESULT_ERRMSG;
				}
				pUUdecode->SetMode();

				for(nLen = pCurrentPart->NextLine(szBuffer, stm);
					nLen > -1;
					nLen = pCurrentPart->NextLine(szBuffer, stm))
				{
					if(bMultipart)
					{
						if(nLen == 0)
							break;
						if(strcmp(szBuffer.chars(), pBoundary) == 0)
							break;
					}

					if(strcmp(szBuffer.chars(), "end") == 0)
						break;
					pUUdecode->Decode(szBuffer);
				}
				pUUdecode->Close();
				break;
			}
		}
		if(pUUdecode == NULL)
			return RESULT_NONE;			// no uu decode part
		delete pUUdecode;
	}

	if(m_DecodeName.length() == 0)
		return RESULT_NONE;				// No filename

	ZString stgName = DECODEDIR + m_stgHashName;
	FILE* pFile = NULL;
	if( !(pFile = fopen(stgName, "r")) )
		return RESULT_ERRASSERT;
	if (fclose(pFile) == EOF)
		return RESULT_ERRASSERT;

	StoreCacheFile(stm, 1);
	UpdateDB(stm);
	UnLockFile(LockFileName, &m_nLockFile);

	OutLocation(stm, m_stgHashName);
	return RESULT_OK;
}

void NewsDecode::DecodeUU (ostream & stm, ArticleSpool & artspool)
{
	char * p;
	if (m_nMulti == MULTI_UUE)
	{
		MultiUUEDecode(stm);
		return;
	}

	ZString stgPattern;
	if( !m_nMulti && 
		m_Subject.IsMultiPart(stgPattern, &m_nTotalPart, &m_nPartNo) && 
		(m_nTotalPart > 1) && m_nPartNo > 0 )
	{
		// Multiple parts
		m_nMulti = MULTI_UUE;
		if(!OpenOver())
		{
			stm << "Content-Type: text/html" << endl << endl
				<< DEF_BODY_TAG << endl
				<< "<h2>Cannot open " << HTMLText(m_stgGroup)
				<< " Group List to complete muli-parts file</h2>"
				<< endl;
			return;
		}

		int nSize = m_nTotalPart + 1;
		m_pSubjects = new ZString[nSize];
		m_pNames = new ZString[nSize];
		m_pArtNums = new ZString[nSize];
		m_pArtSizes = new int[nSize];

#ifdef TRACE
		cerr << "DecodeUU " << stgPattern;
#endif

		int i, nPart, nCount=0;
		ZString tmpString;
		do
		{
			p = m_pfileOver->GetLine();
			int numArticle = atoi(p);				// 1st = Article Number
			if(numArticle == 0)
				continue;

			char * szArtNum = p;
			while(*p && *p != '\t') p++;
			*p++ = 0;

			char * q = p;							// 2nd = Subject
			while(*p && *p != '\t') p++;	
			*p++ = 0;
			NewsSubject Subject = q;

			tmpString = q;
			if(Subject.IsMultiPart(tmpString, NULL, &nPart) &&
				(nPart > 0) &&
				(nPart <= m_nTotalPart) &&
#ifdef TRACE
				(cerr << tmpString << endl, tmpString == stgPattern))
#else
				(tmpString == stgPattern))
#endif
			{
				if( (m_pArtNums[nPart].length()<1) && nPart )
					nCount++;

				m_pSubjects[nPart] = Subject.stg();
				m_pArtNums[nPart] = szArtNum;

				q = p;			// 3rd = Author
				while(*p && *p != '\t') p++;
				*p++ = 0;
				INMailName(q).RealName(m_pNames[nPart]);

				for(i = 3; i < 7; i++)	// 7th = no. of line
				{
					while(*p && *p != '\t') p++;
					p++;
				}
				m_pArtSizes[nPart] = atoi(p);
			}

			if (nCount == m_nTotalPart)
				break;
		} while(m_pfileOver->NextLine());

		for(i = 1; i <= m_nTotalPart; ++i)
		{
			if(m_pArtNums[i].length() == 0)
				break;
			if(i == m_nTotalPart)
			{
				MultiUUEDecode(stm);
				return;
			}
		}

		Display_missing(stm);
		return;
	}

	//
	// Look for begin line
	//
	int nResult;
	while((p = artspool.GetLine()) != NULL)
	{
		if(*p == '\0')
			continue;
		if(FindUUBegin(stm, p))
		{
			if(FindCache(stm))
				return;				// Found file in cache

			CreateCacheName(stm, m_DecodeName);

			// Start the UU decode
			UUDecode uudecode;
			if(!uudecode.Open(m_stgHashName))
			{
				stm << "Content-Type: text/html" << endl << endl
					<< DEF_BODY_TAG << endl
					<< "<h2>Cannot create the decoded file "
					<< HTMLText(m_stgHashName)
					<< "</h2>" << endl;
				return;
			}
			uudecode.SetMode();

			while((p = artspool.GetLine()) != NULL)
			{
				if(strcmp(p, "end") == 0)
					break;				// done
				uudecode.Decode(p);
			}
			uudecode.Close();

			//
			// Make sure we can open the decoded file
			//
			ZString FileName = DECODEDIR + m_stgHashName;
			FILE * pFile;
			if(!(pFile=fopen(FileName, "r")))
				stm << "Content-Type: text/html" << endl << endl
					<< DEF_BODY_TAG << endl
					<< "<h2>Decode format is not recognized</h2>" << endl;
			else
			{
				fclose(pFile);
				StoreCacheFile(stm, 1);
				UpdateDB(stm);
				UnLockFile(LockFileName, &m_nLockFile);

				OutLocation(stm, m_stgHashName);
			}
			return;
		}
		else if (strncmp(p, "(This file", 10) == 0)
		{
			// found HexBin begin
			ZHexBin HexBin;
			if(!HexBin.Init(&artspool, m_DecodeName, 1, NULL, NULL))
				break;				// Reach end of article

			if(FindCache(stm))
				return;				// Found file in cache

			CreateCacheName(stm, m_DecodeName);

			if(!HexBin.Open(m_stgHashName))
			{
				stm << "Content-Type: text/html" << endl << endl
					<< DEF_BODY_TAG << endl
					<< "<h2>Cannot create the decoded file "
					<< HTMLText(m_stgHashName)
					<< "</h2>" << endl;
				return;
			}

			nResult = HexBin.Decode(&artspool);
			ZString FileName = DECODEDIR + m_stgHashName;
			if(nResult >= 0 || nResult == HB_EOF)
			{
				//
				// Make sure we can open the decoded file
				//
				FILE * pFile;
				if(!(pFile=fopen(FileName, "r")))
					stm << "Content-Type: text/html" << endl << endl
						<< DEF_BODY_TAG << endl
						<< "<h2>Decode format is not recognized</h2>" << endl;
				else
				{
					fclose(pFile);
					StoreCacheFile(stm, 1);
					UpdateDB(stm);
					UnLockFile(LockFileName, &m_nLockFile);

					OutLocation(stm, m_stgHashName);
				}
				return;
			}
			else if(nResult == HB_ERR_CRC)
			{
				remove(FileName);
				UnLockFile(LockFileName, &m_nLockFile);
				stm << "Content-Type: text/html" << endl << endl
					<< DEF_BODY_TAG << endl
					<< "<h2>Cannot decode article (CRC Error)</h2>" << endl;
				return;
			}
			break;
		}
	}
	stm << "Content-Type: text/html" << endl << endl
		<< DEF_BODY_TAG << endl
		<< "<h2>Cannot decode article</h2>" << endl;
}

void NewsDecode::MultiUUEDecode (ostream & stm)
{
	m_DecodeName = "";
	ZString DecodeName;
	UUDecode * pUUdecode = NULL;
	ZHexBin * pHexBin = NULL;
	ArticleSpool * partspool = NULL;
	char * p, * q;
	BOOL bInBegin = FALSE;
	int i, n, nResult;
	m_pMessageIDs = new ZString[m_nTotalPart+1];
	for(i = 1; i <= m_nTotalPart; i++)
	{
		if (m_pArtNums[i].length() < 1)
			continue;
		partspool = new ArticleSpool(m_stgGroup, m_pArtNums[i].chars());
		if(!partspool || !partspool->good())
		{
			delete partspool;
			stm << "Content-Type: text/html" << endl << endl
				<< DEF_BODY_TAG << endl
				<< "<h2>Article " << HTMLText(m_stgGroup) << "/"
				<< HTMLText(m_pArtNums[i]) << " not exist</h2>" << endl;
			return;
		}

		partspool->NoFile();

		//
		// Throw away header
		//
		while((p = partspool->GetLine()) != NULL)
		{
			if(*p == '\0')
				break;
			else if(strncasecmp(p, "message-id:", 11) == 0)
			{
				p += 11;
				while(*p && (*p == ' ' || *p == '\t')) p++;
				if(*p && *p == '<') p++;
				m_pMessageIDs[i] = p;
				if(m_pMessageIDs[i].lastchar() == '>')
				{
					n = m_pMessageIDs[i].length() - 1;
					m_pMessageIDs[i].del(n, 1);
				}
			}
		}

		int decoded = false;
		while((p = partspool->GetLine()) != NULL)
		{
			if(*p == 0)
			{
				if(decoded)
					break;
				continue;
			}

			if(pHexBin == NULL && FindUUBegin(stm, p))
			{
				if(pUUdecode != NULL)
					break;					// 2nd pictures - done

				m_MessageID = m_pMessageIDs[1];
				if(FindCache(stm))
				{
					delete partspool;
					return;
				}

				CreateCacheName(stm, m_DecodeName);

				pUUdecode = new UUDecode;
				if(!pUUdecode->Open(m_stgHashName))
				{
					delete partspool;
					stm << "Content-Type: text/html" << endl << endl
						<< DEF_BODY_TAG << endl
						<< "<h2>Cannot create the decoded file "
						<< HTMLText(m_stgHashName)
						<<  "</h2>" << endl;
					delete pUUdecode;
					return;
				}
				pUUdecode->SetMode();
				bInBegin = TRUE;
				continue;
			}
			else if(pUUdecode == NULL && strncmp(p, "(This file", 10) == 0)
			{
				if(pHexBin != NULL)
					break;				// 2nd picture, done

				// found HexBin begin
				pHexBin = new ZHexBin;
				if(!pHexBin->Init(partspool, m_DecodeName, m_nTotalPart,
						m_pArtNums, m_stgGroup))
				{
					stm << "Content-Type: text/html" << endl << endl
						<< DEF_BODY_TAG << endl
						<< "<h2>Cannot decoded the article "
						<< HTMLText(m_stgHashName)
						<<  "</h2>" << endl;
					delete partspool;
					delete pHexBin;
					return;
				}

				m_MessageID = m_pMessageIDs[1];
				if(FindCache(stm))
				{
					delete partspool;
					return;				// Found file in cache
				}

				CreateCacheName(stm, m_DecodeName);

				if(!pHexBin->Open(m_stgHashName))
				{
					stm << "Content-Type: text/html" << endl << endl
						<< DEF_BODY_TAG << endl
						<< "<h2>Cannot create the decoded file "
						<< HTMLText(m_stgHashName)
						<< "</h2>" << endl;
					delete partspool;
					delete pHexBin;
					return;
				}

				nResult = pHexBin->Decode(partspool);
				DecodeName = DECODEDIR + m_stgHashName;
				if(nResult >= 0 || nResult == HB_EOF)
				{
					FILE * pFile;
					if(!(pFile = fopen(DecodeName, "r")))
						stm << "Content-Type: text/html" << endl << endl
							<< DEF_BODY_TAG << endl
							<< "<h2>Decode format is not recognized</h2>"
							<< endl;
					else
					{
						fclose(pFile);
						StoreCacheFile(stm, m_nTotalPart);
						for(i = 1; i <= m_nTotalPart; i++)
						{
							m_MessageID = m_pMessageIDs[i];
							UpdateDB(stm);
						}
						UnLockFile(LockFileName, &m_nLockFile);

						OutLocation(stm, m_stgHashName);
					}
				}
				else
				{
					remove(DecodeName);
					UnLockFile(LockFileName, &m_nLockFile);
					stm << "Content-Type: text/html" << endl << endl
						<< DEF_BODY_TAG << endl;
					if(nResult == HB_ERR_PART)
					{
						stm << "<h2>Article " << HTMLText(m_stgGroup) << "/"
							<< HTMLText(m_pArtNums[pHexBin->GetCurPart()])
							<< " not exist</h2>" << endl;
					}
					else if(nResult == HB_ERR_CRC)
					{
						stm << "<h2>Cannot decode article (CRC Error)</h2>"
							<< endl;
					}
					else
					{
						stm << "<h2>Cannot decoded the article "
							<< HTMLText(m_stgHashName)
							<< "</h2>" << endl;
					}
				}
				delete partspool;
				delete pHexBin;
				return;
			}

			if(!bInBegin)
			{
				// Not in begin - look for it
				if(pHexBin == NULL && strncasecmp(p, "begin", 5) == 0)
				{
					if(pUUdecode)
						bInBegin = TRUE;
				}
				else if(pUUdecode == NULL && strcmp(p, "---") == 0)
				{
					if(pHexBin)
					{
						bInBegin = TRUE;
						nResult = pHexBin->Decode(partspool);
						if(nResult == HB_EOF)
							bInBegin = FALSE;
						else if(nResult == HB_ERR_OPEN || nResult == HB_ERR_CRC)
						{
							delete partspool;
							stm << "Content-Type: text/html" << endl << endl
								<< DEF_BODY_TAG << endl
								<< "<h2>Cannot decoded the article "
								<< HTMLText(m_stgHashName)
								<<  "</h2>" << endl;
							delete pHexBin;
							return;
						}
					}
				}
				continue;
			}

			if(pHexBin)
				break;

			//
			// In begin - look for end
			//
			if(strncasecmp(p, "end", 3) == 0)
			{
				bInBegin = FALSE;
				continue;
			}

			//
			// In begin - process statements
			//
			q = p;
			while(*q && (*q == '-' || *q == ' ')) q++;
			if(*q != '\0' && strncasecmp(p, "section", 7) != 0 &&
				strncasecmp(q, "sum", 3) != 0 &&
				strncasecmp(q, "begin", 5) != 0 &&
				strncasecmp(q, "cut here", 8) != 0)
			{
				decoded = true;
				pUUdecode->Decode(p);
			}
		}
		delete partspool;
	}
	if(pUUdecode)
	{
		pUUdecode->Close();
		delete pUUdecode;

		DecodeName = DECODEDIR + m_stgHashName;
		FILE * pFile;
		if(!(pFile = fopen(DecodeName, "r")))
			stm << "Content-Type: text/html" << endl << endl
				<< DEF_BODY_TAG << endl
				<< "<h2>Decode format is not recognized</h2>" << endl;
		else
		{
			fclose(pFile);
			StoreCacheFile(stm, m_nTotalPart);
			for(i = 1; i <= m_nTotalPart; i++)
			{
				m_MessageID = m_pMessageIDs[i];
				UpdateDB(stm);
			}
			UnLockFile(LockFileName, &m_nLockFile);

			OutLocation(stm, m_stgHashName);
		}
	}
	else
	{
		stm << "Content-Type: text/html" << endl << endl
			<< DEF_BODY_TAG << endl
			<< "<h2>Cannot recognize the decode format</h2>"
			<< endl;
	}
}

BOOL NewsDecode::FindPart (ostream &,  char * p)
{
	m_nPartNo = 1;
	m_nTotalPart = 1;

	char * q, * s;
	BOOL bFoundDigit;
	while((q = strchr(p, '/')) != NULL)
	{
		// Find a slash, try to see if there is a part no before it
		s = q;
		q--;
		while(q >= p && isspace(*q)) q--;	// chop the space
		bFoundDigit = FALSE;
		while(q >= p && isdigit(*q))
		{
			q--;
			bFoundDigit = TRUE;
		}
		if(!bFoundDigit)
		{
			// No digit before slash, not a valid part string, continue with
			//	string after the slash
			p = s + 1;
			continue;
		}
		int nPart = atoi(q+1);

		// Now check if there is a totol part after slash
		q = s + 1;
		while(*q && isspace(*q)) q++;		// chop the space
		p = q;
		bFoundDigit = FALSE;
		while(*q && isdigit(*q))
		{
			q++;
			bFoundDigit = TRUE;
		}

		if(!bFoundDigit)
		{
			// No digit after slash, not a valid part string, continue with
			//	string after the slash
			continue;
		}

		m_nTotalPart = atoi(p);
		m_nPartNo = nPart;
		return TRUE;
	}
	return FALSE;
}

BOOL NewsDecode::FindUUBegin (ostream &, const char * p)
{
	if(strncmp(p, "begin", 5) != 0)
		return FALSE;

	p += 5;
	if(!isspace(*p)) return FALSE;

	while(*p && isspace(*p)) p++;		// skip the extra space

	if(p[0] < '0' || p[0] > '7' || p[1] < '0' || p[1] > '7' ||
		p[2] < '0' || p[2] > '7')
		return FALSE;					// invalid mode

	if(p[0] == '0' && (p[3] >= '0' || p[3] <= '7'))
		p++;

	if(!isspace(p[3])) return FALSE;
	p += 3;
	while(*p && isspace(*p)) p++;		// skip the space

	// Get file name
	if(*p == '~') p++;
	m_DecodeName = "";
	while(*p)
	{
		// Convert all illegal chars in filename to underscore
		if(*p < 0)
			m_DecodeName += *p;
		else
			m_DecodeName += NameTbl[*p];
		p++;
	}
	if(m_DecodeName.length() > 0)
		return TRUE;

	GetNameFromSubject(m_DecodeName);
	if(m_DecodeName.length() > 0)
		return TRUE;

	return FALSE;
}

ZString& NewsDecode::GetNameFromSubject (ZString& DecodeName)
{
	const char * p = m_Subject.stg().chars();
	const char * q, * s;
	int nExt, nName;
	while((q = strchr(p, '.')))
	{
		nExt = 1;
		s = q+1;

		// Get length of extension part
		while(*s && !isspace(*s) && *s != '.')
		{
			nExt++;
			s++;
		}

		if(nExt <= 1)
		{
			p = s;
			continue;
		}

		// Found extension, get the name
		nName = 0;
		s = q-1;
		while(s >= p && !isspace(*s))
		{
			nName++;
			s--;
		}
		s++;

		if(nName == 0)
		{
			p = q + 1;
			continue;
		}

		// Found name
		nName += nExt;
		ZString name(s, nName);
		DecodeName = name;
		return DecodeName;
	}

	DecodeName = "";
	return DecodeName;
}

BOOL NewsDecode::OpenWriteFile (ostream&)
{
	pid_t pid = getpid();
	char szpid[7];
	sprintf(szpid, "%06d", pid);
	szpid[6] = '\0';
	m_DecodeName = TEMPDECODEDIR;
	m_DecodeName += szpid;;
	if( !(m_pFile = fopen(m_DecodeName, "w")) )
		return FALSE;
	return TRUE;
}

void NewsDecode::CloseWriteFile ()
{
	fclose(m_pFile);
	m_pFile = NULL;
}

int NewsDecode::DecodeWriteFile (ostream& stm)
{
	NewsDecode* pDecode = new NewsDecode(m_DecodeName, -1, FALSE);
	if (!pDecode)
		return RESULT_ERRASSERT;
	pDecode->m_stgGroup = m_stgGroup;
	pDecode->m_dirGroup = m_dirGroup;

	struct stat sb;
	if((stat(m_DecodeName, &sb) < 0) || ((sb.st_mode & S_IFMT) == S_IFDIR))
	{
		stm << "Content-Type: text/html" << endl << endl
			<< DEF_BODY_TAG << endl
			<< "<h2>Missing decoding file</h2>" << endl;
		remove(m_DecodeName);
		delete pDecode;
		return RESULT_ERRMSG;
	}

	ArticleSpool artspool(m_DecodeName);
	if( !artspool.good() )
	{
		stm << "Content-Type: text/html" << endl << endl
			<< DEF_BODY_TAG << endl
			<< "<h2>Missing decoding file</h2>" << endl;
		remove(m_DecodeName);
		delete pDecode;
		return RESULT_ERRMSG;
	}

	char * p;
	while((p = artspool.GetLine()) != NULL)
	{
		if( (*p == 0) || (*p == '\r') )
			break;
		if (strncasecmp(p, "subject:", 8) == 0)
		{
			p += 8;
			while( *p && (*p == ' ' || *p == '\t') )	p++;
			pDecode->m_Subject.stg() = p;
		}
		else if (strncasecmp(p, "mime-version:", 13) == 0)
		{
			pDecode->m_NewsMIME.SetMIME(TRUE);
		}
	}
	if (pDecode->m_Subject.stg().length() <= 0)
	{
		stm << "Content-Type: text/html" << endl << endl
			<< DEF_BODY_TAG << endl
			<< "<h2>Empty Article</h2>" << endl;
		remove(m_DecodeName);
		delete pDecode;
		return RESULT_ERRMSG;
	}

	pDecode->m_nMulti = m_nMulti;
	int n = RESULT_OK;
	if (pDecode->m_NewsMIME.IsMIME())
	{
		if(pDecode->m_NewsMIME.Parse(stm, m_DecodeName))
		{
			pDecode->m_MessageID = m_MessageID;
			pDecode->DecodeMIME(stm);
		}
		else
		{
			stm << "Content-Type: text/html" << endl << endl
				<< DEF_BODY_TAG << endl
				<< "<h2>Invalid MIME format</h2>" << endl;
			remove(m_DecodeName);
			delete pDecode;
			return RESULT_ERRMSG;
		}
	}
	else
	{
		pDecode->DecodeUU(stm, artspool);
	}

	//
	// Loop through all the parts and update the database
	//
	m_stgHashName = pDecode->m_stgHashName;
	m_bCache = pDecode->m_bCache;

	StoreCacheFile(stm, m_nTotalPart);
	for(int i = 1; i <= m_nTotalPart; i++)
	{
		if(m_pArtNums[i].length() == 0)
			continue;
		m_MessageID = m_pMessageIDs[i];
		UpdateDB(stm);
	}
	OutLocation(stm, m_stgHashName);
	remove(m_DecodeName);
	delete pDecode;
	return n;
}

int NewsDecode::MultiMIMEDecode (ostream& stm)
{
	int i, n, nLen;
	ZString szBuffer;
	ZString NewsGroup = m_stgGroup + "/";
	for( i=1; (i<=m_nTotalPart) && (m_pArtNums[i].length()<1); i++ )
	{}
	if (i > m_nTotalPart)
		return RESULT_ERRASSERT;
	if (!OpenWriteFile(stm))
		return RESULT_ERRASSERT;
	BOOL bGetMesg = FALSE;
	if (!m_pMessageIDs)
	{	
		m_pMessageIDs = new ZString[m_nTotalPart+1];
		if (!m_pMessageIDs)
			return RESULT_ERRASSERT;
		bGetMesg = TRUE;
	}
	ZString FileName = NewsGroup + m_pArtNums[i];
	NewsDecode* pArt = new NewsDecode(FileName, -1, FALSE);
	if (!pArt)
	{
		CloseWriteFile();
		return RESULT_ERRASSERT;
	}
	pArt->m_stgGroup = m_stgGroup;
	pArt->m_dirGroup = m_dirGroup;
	ArticleSpool artspool(m_stgGroup, m_pArtNums[i]);
	if (!artspool.good())
	{
		CloseWriteFile();
		return RESULT_ERRASSERT;
	}
	if (!pArt->m_NewsMIME.Parse(stm, artspool.File()))
	{
		CloseWriteFile();
		delete pArt;
		return RESULT_ERR;
	}
	MIMEBodyPart* pCurrentPart = pArt->m_NewsMIME.FirstPart(TRUE);
	if (!pCurrentPart)
	{
		CloseWriteFile();
		delete pArt;
		return RESULT_ERRASSERT;
	}
	if (!pCurrentPart->Open())
	{
		CloseWriteFile();
		delete pArt;
		return RESULT_ERRASSERT;
	}
	for( nLen = pCurrentPart->FirstLine(szBuffer, stm); 
		nLen > 0; 
		nLen = pCurrentPart->NextLine(szBuffer, stm) )
	{
		if( !strncasecmp(szBuffer, "Content-", 8) || 
			pCurrentPart->HeaderCompare(szBuffer, "Encrypted") || 
			pCurrentPart->HeaderCompare(szBuffer, "Lines") )
		{
			while (szBuffer.lastchar() == ';')
				pCurrentPart->NextLine(szBuffer, stm);
			continue;
		}
		else
		{
			szBuffer += "\n";
			n = fwrite(szBuffer, sizeof(char), ++nLen, m_pFile);
			if (n != nLen)
			{
				CloseWriteFile();
				pCurrentPart->Close();
				delete pArt;
				return RESULT_ERRASSERT;
			}
		}
	}
	if (i != 1)
		fwrite("\n", sizeof(char), 1, m_pFile);
	for( nLen = pCurrentPart->NextLine(szBuffer, stm); 
		nLen > -1; 
		nLen = pCurrentPart->NextLine(szBuffer, stm) )
	{
		szBuffer += "\n";
		n = fwrite(szBuffer, sizeof(char), ++nLen, m_pFile);
		if (n != nLen)
		{
			CloseWriteFile();
			pCurrentPart->Close();
			delete pArt;
			return RESULT_ERRASSERT;
		}
	}
	if (bGetMesg)
	{
		m_pMessageIDs[i] = pArt->m_NewsMIME.MessageID();
		const char* z = m_pMessageIDs[i].chars();
		z += 11;
		while( *z && (*z == ' ' || *z == '\t') ) z++;
		if( *z && (*z == '<') ) z++;
		m_pMessageIDs[i] = z;
		if (m_pMessageIDs[i].lastchar() == '>')
		{
			int n = m_pMessageIDs[i].length() - 1;
			m_pMessageIDs[i].del(n, 1);
		}
	}
	pCurrentPart->Close();
	delete pArt;

	for( ++i; i<=m_nTotalPart; i++ )
	{
		if (m_pArtNums[i].length()<1)
			continue;
		FileName = NewsGroup + m_pArtNums[i];
		pArt = new NewsDecode(FileName, -1, FALSE);
		if (!pArt)
		{
			CloseWriteFile();
			return RESULT_ERRASSERT;
		}
		pArt->m_stgGroup = m_stgGroup;
		pArt->m_dirGroup = m_dirGroup;
		ArticleSpool artspool(m_stgGroup, m_pArtNums[i]);
		if (!artspool.good())
		{
			CloseWriteFile();
			return RESULT_ERRASSERT;
		}
		if (!pArt->m_NewsMIME.Parse(stm, artspool.File()))
		{
			CloseWriteFile();
			delete pArt;
			return RESULT_ERR;
		}
		pCurrentPart = pArt->m_NewsMIME.FirstPart(TRUE);
		if (!pCurrentPart)
		{
			CloseWriteFile();
			delete pArt;
			return RESULT_ERRASSERT;
		}
		if (!pCurrentPart->Open())
		{
			CloseWriteFile();
			delete pArt;
			return RESULT_ERRASSERT;
		}
		for( nLen = pCurrentPart->FirstLine(szBuffer, stm); 
			nLen > 0; 
			nLen = pCurrentPart->NextLine(szBuffer, stm) )
		{}
		for( nLen = pCurrentPart->NextLine(szBuffer, stm); 
			nLen > -1; 
			nLen = pCurrentPart->NextLine(szBuffer, stm) )
		{
			szBuffer += "\n";
			n = fwrite(szBuffer, sizeof(char), ++nLen, m_pFile);
			if (n != nLen)
			{
				CloseWriteFile();
				pCurrentPart->Close();
				delete pArt;
				return RESULT_ERRASSERT;
			}
		}
		if (bGetMesg)
		{
			m_pMessageIDs[i] = pArt->m_NewsMIME.MessageID();
			const char* z = m_pMessageIDs[i].chars();
			z += 11;
			while( *z && (*z == ' ' || *z == '\t') ) z++;
			if( *z && (*z == '<') ) z++;
			m_pMessageIDs[i] = z;
			if (m_pMessageIDs[i].lastchar() == '>')
			{
				int n = m_pMessageIDs[i].length() - 1;
				m_pMessageIDs[i].del(n, 1);
			}
		}
		pCurrentPart->Close();
		delete pArt;
	}

	CloseWriteFile();
	return DecodeWriteFile(stm);
}

DBM * NewsDecode::OpenDB (ostream &, char * pName)
{
	ZString stgTemp = DRNCACHEDIR;
	stgTemp += pName;
	mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
	DBM * pDB = dbm_open(stgTemp, O_RDWR, mode);
  	if(pDB == NULL)
		pDB = dbm_open(stgTemp, O_RDWR|O_CREAT|O_TRUNC, mode);
	return pDB;
}

BOOL NewsDecode::CacheExist (ostream& stm)
{
	DBM * pMsgDB;
	if((pMsgDB = OpenDB(stm, szMessageID)) == NULL)
	{
		DMSG(0, "Cannot opendb %s%s", DRNCACHEDIR, szMessageID);
		return FALSE;				// Can't open the messageid database
	}

	char pBuf[1];
	datum msgdatum, filedatum;
	msgdatum.dptr = (char *)m_MessageID.chars();
	msgdatum.dsize = m_MessageID.length();
	pBuf[0] = '\0';
	filedatum.dptr = pBuf;
	filedatum.dsize = 0;
	filedatum = dbm_fetch(pMsgDB, msgdatum);
	dbm_close(pMsgDB);
	if(filedatum.dsize == 0)
		// No message entry
		return FALSE;
	ZString stg(filedatum.dptr, filedatum.dsize);
	if(stg.contains(m_stgHashName))
		return TRUE;
	return FALSE;
}

BOOL NewsDecode::FindCache (ostream& stm)
{
	ZString decodename = m_DecodeName;
	int idx;
	while(decodename.length() && decodename.contains('/'))
	{
		idx = decodename.index('/');
		decodename = decodename.after(idx);
	}
	if(decodename.length() == 0)
		return FALSE;

	m_DecodeName = decodename;

	LockFileName = m_DecodeName;
	LockFile(stm, LockFileName.chars(), &m_nLockFile);

	DBM * pMsgDB;
	if((pMsgDB = OpenDB(stm, szMessageID)) == NULL)
		return FALSE;				// Can't open the messageid database

	int nBuf = 256+1;
	char * pBuf = new char[nBuf];
	datum msgdatum, filedatum;
	msgdatum.dptr = (char *)m_MessageID.chars();
	msgdatum.dsize = m_MessageID.length();
	pBuf[0] = '\0';
	filedatum.dptr = pBuf;
	filedatum.dsize = 0;
	filedatum = dbm_fetch(pMsgDB, msgdatum);
	if(filedatum.dsize == 0)
	{
		// No message entry
		dbm_close(pMsgDB);
		delete [] pBuf;
		return FALSE;
	}

	if(nBuf < (filedatum.dsize+1))
	{
		delete [] pBuf;
		nBuf = filedatum.dsize + 1;
		pBuf = new char(nBuf);
	}
	strncpy(pBuf, filedatum.dptr, filedatum.dsize);
	pBuf[filedatum.dsize] = '\0';
	char * p = pBuf;
	if(*p == '/')
		p++;
	char * q, * s;
	while(p && *p)
	{
		q = strchr(p, '\t');
		if(q)
			*q++ = '\0';
		s = strchr(p, '/');
		s++;
		if(strcmp(s, m_DecodeName) == 0)
		{
			// Found the decoded filename in database
			ZString stgTemp = DECODEDIR;
			stgTemp += "/";
			stgTemp += p;
			FILE * pFile;
			if(!(pFile = fopen(stgTemp, "r")))
			{
				// Can't open the decoded file, remove it from the database
				if(q && *q)
					strcpy(pBuf, q);
				else
					pBuf[0] = 0;
				filedatum.dptr = pBuf;
				filedatum.dsize = strlen(pBuf);
				if(filedatum.dsize > 0)
					dbm_store(pMsgDB, msgdatum, filedatum, DBM_REPLACE);
				else
					dbm_delete(pMsgDB, msgdatum);
				delete [] pBuf;
				dbm_close(pMsgDB);
				return FALSE;			// Can't open file, re-do the decoding
			}

			fclose(pFile);
			dbm_close(pMsgDB);
			UnLockFile(LockFileName, &m_nLockFile);

			m_bCache = TRUE;
			m_stgHashName = "/";
			m_stgHashName += p;
			OutLocation(stm, m_stgHashName);
			delete [] pBuf;
			return TRUE;
		}
		p = q;
	}
	delete [] pBuf;

	// No entry for the filenam
	return FALSE;
}

void NewsDecode::UnLockFile (const char * pName, int * pLock)
{
	if(*pLock != -1 && *pName)
	{
		pid_t pid;
		int fd;
		ZString stgTemp = LOCKDECODEDIR;
		stgTemp += pName;
		if((fd = open(stgTemp, O_RDONLY, 0644)) > 0)
		{
			read(fd, (char *)(&pid), sizeof(pid_t));
			close(fd);
			if(pid == getpid())
				unlink(stgTemp);
		}
		*pLock = -1;
	}
}

void NewsDecode::LockFile (ostream &, const char * pName, int * pLock)
{
	::InitSignals();
	ZString stgTemp = LOCKDECODEDIR;
	stgTemp += pName;
	int fd;
	pid_t filepid;
	pid_t pid = getpid();
	while((fd = open(stgTemp, O_RDWR|O_CREAT|O_EXCL, 0644)) < 0)
	{
		// file locked by another proces, check if process is still alived
		if((fd = open(stgTemp, O_RDONLY, 0644)) >= 0)
		{
			read(fd, (char *)(&filepid), sizeof(pid_t));
			close(fd);
			if(filepid == pid)
				return;				// locked by current process, done
			if(kill(filepid, 0) < 0)
			{
				// process that locked the file is no longer exist,
				//	unlink the file
				unlink(stgTemp);
			}
		}
		sleep(2);				// File is locked by other process, wait
	}
	write(fd, (char *)(&pid), sizeof(pid_t));
	close(fd);
	*pLock = fd;

}

void NewsDecode::CreateCacheName (ostream &, const char * pName)
{
	setpriority(PRIO_PROCESS, 0, 5);	// lower priority

	ZString stgTemp;
	if(m_bForceDecode)
	{
		m_stgHashName = "/000/" + m_DecodeName;
		stgTemp = DECODEDIR + m_stgHashName;
		remove(stgTemp);
		return;
	}
	stgTemp = m_MessageID + pName;
	int j = 0;
	const char * p = stgTemp.chars();
	NGH_HASH(p, j);
	j &= 255;
	char szDir[6];
	int i = j;
	int nClean = -1;
	long tClean = 0;
	struct stat sb;
	int fd;
	do
	{
		stgTemp = DECODEDIR;
		stgTemp += szDir + m_DecodeName;
		sprintf(szDir, "/%03u/", i+1);
		if((fd = open(stgTemp, O_RDONLY)) < 0)
		{
			m_stgHashName = szDir + m_DecodeName;
			return;
		}
		if(fstat(fd, &sb) > -1) 
		{
#ifdef FREEBSD
			if(nClean < 0 || tClean > sb.st_atimespec.tv_sec)
			{
				nClean = i;
				tClean = sb.st_atimespec.tv_sec;
			}
#else
			if(nClean < 0 || tClean > sb.st_atime)
			{
				nClean = i;
				tClean = sb.st_atime;
			}
#endif
		}
		close(fd);
		i = (i + 1) & 255;
	} while(i != j);

	// filename exist in all buckets, just dump it into the 000 bucket
	m_stgHashName = "/000/" + m_DecodeName;
	stgTemp = DECODEDIR + m_stgHashName;
	remove(stgTemp);
}

void NewsDecode::UpdateDB (ostream & stm)
{
	if(m_MessageID.length() < 1 || m_bForceDecode || m_bCache)
		return;					// empty message id

	if(strncmp(m_stgHashName.chars(), "/000", 4) == 0)
		return;

	LockFile(stm, szMessageID, &m_nLockMsg);	// lock the database
	DBM * pMsgDB = OpenDB(stm, szMessageID);
	if(pMsgDB)
	{
		// Clean up the messageid info in the database file
		char pTemp[2];
		datum msgdatum, filedatum;
		msgdatum.dptr = (char *)m_MessageID.chars();
		msgdatum.dsize = m_MessageID.length();
		pTemp[0] = '\0';
		filedatum.dptr = pTemp;
		filedatum.dsize = 0;
		filedatum = dbm_fetch(pMsgDB, filedatum);
		ZString stgData(filedatum.dptr, filedatum.dsize);
		if(!stgData.contains(m_stgHashName))
		{
			// Not exist yet, OK to add
			if(filedatum.dsize > 0)
				stgData += '\t';
			stgData += m_stgHashName;
			filedatum.dptr = (char *)stgData.chars();
			filedatum.dsize = stgData.length();
			dbm_store(pMsgDB, msgdatum, filedatum, DBM_REPLACE);
		}
		dbm_close(pMsgDB);

	}
	UnLockFile(szMessageID, &m_nLockMsg);
}

void NewsDecode::StoreCacheFile (ostream &, int nPart)
{
	if(!m_bSaveCache || m_bForceDecode || m_bCache)
		return;

	if(strncmp(m_stgHashName.chars(), "/000", 4) == 0)
		return;

	int slen = 0;
	char * pCacheBuf = NULL;
	if(nPart == 1)
	{
		slen = m_stgHashName.length() + m_MessageID.length() + 1;
		pCacheBuf = new char[slen];
		sprintf(pCacheBuf, "%s %s", m_stgHashName.chars()+1,
			m_MessageID.chars());
	}
	else
	{
		int nBufLen = 5000;
		pCacheBuf = new char[nBufLen];
		slen = 0;
		char * p = pCacheBuf;
		strcpy(p, m_stgHashName.chars()+1);
		slen = m_stgHashName.length() - 1;
		p += slen;
		for(int i = 1; i <= nPart; i++)
		{
			if(m_pMessageIDs[i].length() == 0)
				continue;
			if((int)(m_pMessageIDs[i].length()+1) > (nBufLen-slen))
			{
				// Not enough space, allocate more
				nBufLen += 5000;
				p = new char[nBufLen];
				if(p == NULL)
				{
					syslog(LOG_ALERT, "DRN decode cache can't allocate temp buffer (%d) %m", nBufLen);
					delete [] pCacheBuf;
					return;
				}
				strncpy(p, pCacheBuf, slen);
				delete [] pCacheBuf;
				pCacheBuf = p;
				p += slen;
			}
			*p++ = ' ';
			strcpy(p, m_pMessageIDs[i]);
			p += m_pMessageIDs[i].length();
			slen += m_pMessageIDs[i].length() + 1;
		}
		if(slen < nBufLen)
		{
			*p++ = 0;
			slen++;
		}
		else
		{
			nBufLen++;
			p = new char[nBufLen];
			if(p == NULL)
			{
				syslog(LOG_ALERT, "DRN decode cache can't allocate temp buffer (%d) %m", nBufLen);
				delete [] pCacheBuf;
				return;
			}
			strncpy(p, pCacheBuf, slen);
			delete [] pCacheBuf;
			pCacheBuf = p;
			pCacheBuf[slen++] = 0;
		}
	}
	if(slen < 1)
	{
		delete [] pCacheBuf;
		return;
	}
	openlog("drn", LOG_PID, LOG_SYSLOG);
	::InitSignals();
	struct CacheFileInfo info;
	int fd, nFree;
	for(;;)
	{
		if((fd = open(DRNCACHEFILE, O_RDWR|O_EXCL, 0644)) < 0) 
		{
			delete [] pCacheBuf;
/*
			if(errno == ENOENT)
			{
				syslog(LOG_ERR, "DRN decode cache: %m");
				return;
			}
			syslog(LOG_ERR, "DRN decode cache: %m");
*/
			return;
		}
#ifdef SOLARIS
		struct flock lock_it;
		int ret;
   		lock_it.l_whence = SEEK_SET;    /* from current point */
   		lock_it.l_start = 0;        /* -"- */
   		lock_it.l_len = 0;          /* until end of file */
   		lock_it.l_type = F_WRLCK;       /* set exclusive/write lock */
   		lock_it.l_pid = 0;
		while((ret = fcntl(fd, F_SETLKW, &lock_it)) < 0 && errno == EINTR)			;
#else
		flock(fd, LOCK_EX);
#endif
		read(fd, (char *)&info, sizeof(CacheFileInfo));
		if(info.MagicNo != CACHE_MAGICNO)
		{
			// bad magic no., assume the cache file is corrupt
#ifdef SOLARIS
			struct flock lock_it;
			int ret;
   			lock_it.l_whence = SEEK_SET;    /* from current point */
   			lock_it.l_start = 0;        /* -"- */
   			lock_it.l_len = 0;          /* until end of file */
   			lock_it.l_type = F_UNLCK;       /* set exclusive/write lock */
   			lock_it.l_pid = 0;
			while((ret = fcntl(fd, F_SETLKW, &lock_it)) < 0 && errno == EINTR)			;
#else
			flock(fd, LOCK_UN);
#endif
			close(fd);
			delete [] pCacheBuf;
			syslog(LOG_ALERT, "DRN decode cache file is corrupt %m");
			return;
		}

		// Check if enough room to add a new entry
		nFree = info.BegOffset - info.EndOffset;
		if(nFree < 1)
			nFree += CACHE_SIZE;
		if(nFree > (int)(slen + 2 * sizeof(int)))
			break;

		// Not enough room, clean the cache file and try again
#ifdef SOLARIS
   		lock_it.l_whence = SEEK_SET;    /* from current point */
   		lock_it.l_start = 0;        /* -"- */
   		lock_it.l_len = 0;          /* until end of file */
   		lock_it.l_type = F_UNLCK;       /* set exclusive/write lock */
   		lock_it.l_pid = 0;
		while((ret = fcntl(fd, F_SETLKW, &lock_it)) < 0 && errno == EINTR)			;
#else
		flock(fd, LOCK_UN);
#endif
		close(fd);
		system(DRNCLEANCACHE);
		sleep(5);
	}

	// write the entry to the cache file
	if((CACHE_SIZE - info.EndOffset) < sizeof(int))
		info.EndOffset = sizeof(CacheFileInfo);
	lseek(fd, info.EndOffset, SEEK_SET);
	write(fd, (char *)&slen, sizeof(int));
	info.EndOffset += sizeof(int);
	int wlen = 0;
	if((CACHE_SIZE - info.EndOffset) < slen)
	{
		// Not enough room to write the whole string, write till end of
		//	cache file and wrap the rest to the beginning
		write(fd, pCacheBuf, (wlen = CACHE_SIZE-info.EndOffset));
		info.EndOffset = sizeof(CacheFileInfo);
		lseek(fd, info.EndOffset, SEEK_SET);
	}
	write(fd, pCacheBuf + wlen, slen - wlen);
	info.EndOffset += (slen - wlen);
	lseek(fd, 0, SEEK_SET);
	write(fd, (char *)&info, sizeof(CacheFileInfo));
#ifdef SOLARIS
	struct flock lock_it;
	int ret;
   	lock_it.l_whence = SEEK_SET;    /* from current point */
   	lock_it.l_start = 0;        /* -"- */
   	lock_it.l_len = 0;          /* until end of file */
   	lock_it.l_type = F_UNLCK;       /* set exclusive/write lock */
   	lock_it.l_pid = 0;
	while((ret = fcntl(fd, F_SETLKW, &lock_it)) < 0 && errno == EINTR)			;
#else
	flock(fd, LOCK_UN);
#endif
	close(fd);

	nFree = info.BegOffset - info.EndOffset;
	if(nFree < 1)
		nFree += CACHE_SIZE;
	if(nFree < CACHE_MIN)
		system(DRNCLEANCACHE);	// cache file space is low, clean it
}

void NewsDecode::OutLocation (ostream& stm, const char * p)
{
	if(m_bForceDecode || m_bSaveCache)
	{
		char * pDigest = NULL;

#ifdef  DIGESTPWD
		char digest[DIGBUFLEN];
		const char * pUser = gUserInfo.GetUserName();
		const char * pHostAddr = gUserInfo.GetHostAddr();
		const char * pHostDomain = gUserInfo.GetHostDomain();
		char * pPasswd = NULL;
		if(pHostDomain && *pHostDomain != 0)
		{
			// ISP connection
			if(pUser)
				pDigest = digest;
		}
		else
		{
			pDigest = digest;
		}
		if(pDigest)
		{
			ZString cmd = LINKDECODEDIR;
			cmd += p;
			const char * pDBName = DBNAME;
			pPasswd = gethashpwd(pUser, pDBName);
			mkdigest(pDigest, pUser, pPasswd, pHostAddr,
				cmd.chars());
		}
#endif

		stm << "Location: " << DECODEDIR_SERVER;
		if(pDigest)
			stm << "/.." << pDigest;
		stm << LINKDECODEDIR << URLText(p) << endl << endl;
	}
}

BOOL
NewsDecode::OpenOver ()
{
	if(m_pfileOver == NULL)
	{
		m_pfileOver = new Zifstream();
		if(m_pfileOver == NULL)
			return FALSE;
	}

	if(m_pfileOver && m_pfileOver->is_open())
		return TRUE;

	ZString ovf(OVERVIEWDIR);
	ovf += "/";
	ovf += m_dirGroup;
	ovf += "/";
	ovf += ".overview";

	struct stat sb;
#ifdef FREEBSD
	struct timespec	mtimeOver;
#else
	time_t mtimeOver;
#endif
	ArtNumIdx * pArtNumIdx;
	int nArtNum;
	for(int retry = 0; retry < 2; retry++)
	{
		m_pfileOver->open(ovf, ios::in|ios::nocreate);
		if(m_pfileOver->good()
			&& fstat(m_pfileOver->rdbuf()->fd(), &sb) >= 0
			&& sb.st_size > 0)
		{
#ifdef FREEBSD
			mtimeOver = sb.st_mtimespec;
			if((time(NULL) - mtimeOver.tv_sec) < OV_DELAY)
#else
			mtimeOver = sb.st_mtime;
			if((time(NULL) - mtimeOver) < OV_DELAY)
#endif
			return TRUE;
		}
		m_pfileOver->close();

		if(retry > 0)
			break;

		if(!MakeOVDirectory(m_dirGroup))
			break;
		
		if(chdir(m_dirGroup) != 0)
		{
			DMSG(0, "Cannot chdir to %s", m_dirGroup.chars());
			break;
		}

		int fd = open(f_artnum_i, O_CREAT|O_WRONLY|O_TRUNC,0664);
		chdir(NEWSINDEX);					// for core dump
		if(fd < 0)
		{
			DMSG(0, "Cannot create %s", f_artnum_i);
			break;
		}

		ZString tovf = ovf;
		tovf += ".tmp";
		ofstream of(tovf);

		int n;
		char * p;
		try
		{
			NNTPOverview art(m_stgGroup);
			pArtNumIdx = new ArtNumIdx [n=art.GetNumArt()];

			streamoff off = 0;
			nArtNum = 0;
			while((p=art.GetLine()) != NULL)
			{
				if(nArtNum < n)	// enlarge array !!
				{
					pArtNumIdx[nArtNum].off = off;
					pArtNumIdx[nArtNum++].artnum = atol(p);
				}
				of << p << '\n';
				off += strlen(p) + 1;
			}
		}
		catch(const char * p)
		{
			DMSG(0, "NNTPOverview: %s", p);
			break;
		}
		catch(int err)
		{
			DMSG(0, "NNTPOverview: &s", strerror(errno));
			break;
		}

		n = nArtNum*sizeof(ArtNumIdx);
		if(write(fd, pArtNumIdx, n) != n)
		{
			DMSG(0, "NNTPOverview: write index error - &s", strerror(errno));
			break;
		}

		close(fd);
		of.close();
		rename(tovf, ovf);
	}

	DMSG(0, "Cannot open overview dir %s", ovf.chars());
	return FALSE;
}
