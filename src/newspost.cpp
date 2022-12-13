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
#include "newspost.h"

#include "artspool.h"

ostream& operator << (ostream& stm, NewsPost& post)
{
	TemplateError tmplerr;
	if(post.m_stgArticle[0] == '<' && post.m_stgArticle.lastchar() == '>')
	{
		//
		// Look up history file to translate ID into a file name
		//
		tmplerr.OutError(stm, "Message ID not support yet");
		return stm;
	}

	ZString stgGroup = post.m_stgArticle;
	post.m_nArticleNo = 0;
	const char * pGroup = stgGroup;
	const char * qGroup = strrchr(pGroup, '/');
	if(qGroup)
	{
		const char * pArt = ++qGroup;
		for(; *qGroup; qGroup++) { if(!isdigit(*qGroup)) break; }
		if(*qGroup == 0)
		{
			post.m_nArticleNo = pArt - pGroup;
			stgGroup[post.m_nArticleNo-1] = 0;
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
	TmplName += "/post.htm";
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

	// Scan for <!--pathlink drn=post -->
	BOOL bDspPost = FALSE;
	char * p;
	do
	{
		p = tmplFile.GetLine();

		if(stgGroup.length() && strncmp(p, "<head>", 6) == 0)
		{
			stm << p << endl
				<< "<script language=javascript>" << endl
				<< "<!--" << endl
				<< "var Newsgoup = '" << stgGroup.c_str() << "';" << endl
				<< "// -->" << endl
				<< "</script>" << endl
			;
			continue;
		}

		if(strncmp(p, "<!--pathlink drn=post -->", 25) == 0)
		{
			bDspPost = TRUE;
			break;
		}

		stm << p << endl;
	} while(tmplFile.NextLine());

	if(bDspPost)
	{
		ZString stgNewsgroups = stgGroup;
		ZString stgSubject, stgFrom, stgMessageID;
		ZString stgDate, stgReference, stgFollowupTo;
		char * q;
		ArticleSpool * partspool = NULL;
		if(post.m_nArticleNo > 0)
		{
			partspool = new ArticleSpool(stgGroup,
				post.m_stgArticle.chars() + post.m_nArticleNo);
			if(!partspool->good())
			{
				stm << "<h3>Article " << post.m_stgArticle << " not found</h3>"
					<< endl << "This article may have been expired" << endl;
				post.OutEnd(stm, stgGroup, tmplFile);
				delete partspool;
				return stm;
			}

			partspool->NoFile();

			// Scan headers
			while((p = partspool->GetLine()))
			{
				if(*p == 0 || *p == '\r')
					break;
				
				q = strchr(p, ' ');
				while(q && *q == ' ')
					q++;
	
				if(strncasecmp(p, "from:", 5) == 0)
				{
					INMailName(q).RealName(stgFrom);
				}
				else if(strncasecmp(p, "newsgroups:", 11) == 0)
				{
					if(!post.m_opt.IsNoCrossPost())
						stgNewsgroups = q;
				}
				else if(strncasecmp(p, "subject:", 8) == 0)
				{
					stgSubject = q;
					if(strncmp(q, "Re:", 3) != 0)
					{
						stgSubject = "Re: " + stgSubject;
					}
				}
				else if(strncasecmp(p, "date:", 5) == 0)
				{
					stgDate = q;
				}
				else if(strncasecmp(p, "Message-ID:", 11) == 0)
				{
					stgMessageID = q;
				}
				else if(strncasecmp(p, "References:", 11) == 0)
				{
					while(strlen(q) > 1000)
					{
						q = strchr(q, ' ');
						while(q && *q == ' ')
							q++;
					}
					stgReference = q;
				}
				else if(strncasecmp(p, "Followup-To:", 12) == 0)
				{
					stgFollowupTo = q;
				}
			}
		}

		if(stgFollowupTo.length() > 0 && !post.m_opt.IsNoCrossPost())
			stgNewsgroups = stgFollowupTo;

		post.OutToolbar(stm, stgGroup.chars(), stgMessageID.chars());

		//
		// Display post
		//
		stm << "<font size=-1 face=\"Arial\">" << endl
			<< "<TITLE>Newsadm News Service - Posting News to"
			<< HTMLText(stgNewsgroups) << "</TITLE><br>" << endl
			<< "<form name=PostForm ";

		if(post.m_bAttach)
			stm << "enctype=multipart/form-data ";

		stm << "method=POST action=\"" << NEWSBIN << "/wwwinews\">" << endl
			<< "<table border=0 width=480>" << endl
			<< "<tr><td width=110><font face=Verdana size=2 color=#000000>"
			<< endl
			<< "<b>Newsgroup:</b><br>" << endl
			<< "</font></td><td>" << endl
			<< "<input type=text name=Newsgrps size=50 value=\""
			<< HTMLText(stgNewsgroups) << "\"><br>" << endl
			<< "</td></tr>" << endl
			<< "<tr><td width=110><font face=Verdana size=2 color=#000000>"
			<< endl
			<< "<b>Subject:</b><br>" << endl
			<< "</font></td><td>" << endl
			<< "<input type=text name=Subject size=50"
		;
//		if(stgSubject.length() > 0)
			stm << " value=\"" << HTMLText(stgSubject) << "\"";
		stm << "><br>" << endl
			<< "</td></tr>" << endl
			<< "<tr><td width=110><font face=Verdana size=2 color=#000000>"
			<< endl
			<< "<b>From:</b><br>" << endl
			<< "</font></td><td>" << endl
			<< "<input type=text name=From size=50";
		;

		ZString stgTmp;
		const char * r = post.m_opt.GetFromName();
		const char * s = post.m_opt.GetFromEmail();
		if(*r && *s)			// got both fromname and fromemail
		{
			stgTmp = r;
			stgTmp += "<";
			stgTmp += s;
			stgTmp += ">";
		}
		else if(*r)				// got fromname
		{
			stgTmp = r;
		}
		else if(*s)				// got fromemail
		{
			stgTmp = s;
		}
		if(stgTmp.length() > 0)
			stm << " value=\"" << HTMLText(stgTmp) << "\"";

		stm << "><br>" << endl
			<< "</td></tr>" << endl
			<< "<tr><td width=110><font face=Verdana size=2 color=#000000>"
			<< endl
			<< "<b>Organization:</b><br>" << endl
			<< "</font></td><td>" << endl
			<< "<input type=text name=Org size=50"
		;
		r = post.m_opt.GetOrganization();
		if(r && *r)
			stm << " value=\"" << HTMLText(r) << "\"";

		stm << "><br>" << endl
			<< "</td></tr>" << endl
			<< "<tr><td width=110><font face=Verdana size=2 color=#000000>"
			<< endl
			<< "<b>Cc: (E-Mail)</b><br>" << endl
			<< "</font></td><td>" << endl
			<< "<input type=text name=ReplyTo size=50><br>" << endl
			<< "</td></tr>" << endl;

		if(post.m_bAttach)
		{
			stm << "<tr><td width=110 valign=middle>" << endl
				<< "<font face=Verdana size=2 color=#000000>" << endl
				<< "<b>Attachment:</b>" << endl
				<< "</font></td><td>" << endl
				<< "<input type=file name=Filename>&nbsp;&nbsp;"
				<< "<font face=Verdana size=2 color=#000000>" << endl
				<< "<a href=\"/drn/drnhelp1.htm\">" << endl
				<< "<img src=/drn/images/posthelp.gif border=0 valign=top alt=Help>"
				<< "</a><br>" << endl
				<< "</font></td></tr>" << endl;
		}
		stm << "</table>" << endl
			<< "<font face=Verdana size=2 color=#000000>" << endl
		;

		if(stgMessageID.length() > 0)
		{
			if(stgReference.length() > 0)
				stgReference += ' ';
			stgReference += stgMessageID;
		}
		if(stgReference.length() > 0)
		{
			stm << "<input type=hidden name=Refer value=\""
				<< HTMLText(stgReference) << "\">" << endl
			;
		}
		r = post.m_opt.GetReplyEmail();
		if(r && *r)
		{
			stm << "<input type=hidden name=ReplyFrom value=\""
				<< HTMLText(r) << "\">" << endl
			;
		}
		stm
//			<< "<strong>Content-Type: text/html</strong>&nbsp;"
//			<< "<input type=checkbox name=TypeHTML><br>" << endl
			<< "<strong>X-No-Archive:</strong>&nbsp;"
			<< "<input type=checkbox name=Archive"
		;
		if(post.m_opt.IsNoXArchive())
			stm << " checked";
		stm << "><p>" << endl
			<< "<input src=/drn/images/posttab1.gif name=Send "
			<< "alt=\"Post Message\" type=image border=0> " << endl
		;

		const char * pUser = gUserInfo.GetUserName();
		if(pUser && *pUser)
		{
			stm << "<a href=\"" << NEWSBIN << "/preference\">" << endl
				<< "<img src=/drn/images/posttab2.gif alt=\"Set My "
				<< "Preferences\" border=0></a>" << endl
			;
		}

		stm
#ifdef SPELLCHECK
			<< "<script language=javascript>" << endl
			<< "<!--" << endl
			<< "document.write('<a href=javascript:SpellCheck()>"
			<< "<img src=/drn/images/spellcheck.gif alt=SpellCheck border=0>"
			<< "</a>');" << endl
			<< "// -->" << endl
			<< "</script>" << endl
#endif
			<< "<br>" << endl
			<< "</font><textarea name=Body rows=15 cols=80 wrap=PHYSICAL>"
		;

		if(post.m_nArticleNo > 0)
		{
			stgTmp = USERDIR;
			stgTmp += gUserInfo.GetUserName();
			stgTmp += "/openquote";
			Zifstream qFile(stgTmp.chars(), ios::nocreate|ios::in);
			ZString stgQuote;
			if(qFile && qFile.good())
			{
				// Read the quote file
				do
				{
					p = qFile.GetLine();
					if(*p == 0 || *p == '\r')
						continue;
				
					stgQuote += p;
				} while(qFile.NextLine());
			}

			if(stgQuote.length() == 0)
				stgQuote = "In article %M%, %F% says...";

			unsigned int n;
			if((n = stgQuote.find("%D%")) != string::npos)
			{
				stgQuote.replace(n, 3, stgDate);
			}
			if((n = stgQuote.find("%F%")) != string::npos)
			{
				stgQuote.replace(n, 3, stgFrom);
			}
			if((n = stgQuote.find("%M%")) != string::npos)
			{
				stgQuote.replace(n, 3, stgMessageID);
			}

			stm << HTMLText(stgQuote) << endl << ">" << endl;

			while((p = partspool->GetLine()))
			{
				if(*p == 0 || *p == '\r')
				{
					stm << ">" << endl;
					continue;
				}
				stm << ">" << HTMLText(p) << endl;
			}
		}

		stm << "</textarea>" << endl
			<< "<p><font face=Verdana size=2 color=#000000>"
			<< "Post Signature (Maximum 80 characters wide, 5 lines)"
			<< "</font><br>" << endl
			<< "<textarea name=Signature rows=5 cols=80 wrap=PHYSICAL>"
		;

		stgTmp = USERDIR;
		stgTmp += gUserInfo.GetUserName();
		stgTmp += "/signature";
		FILE * sFile = fopen(stgTmp.chars(), "r");
		if(sFile)
		{
			char buf[1024];
			while(feof(sFile) == 0)
			{
				p = fgets(buf, 1024-1, sFile);
				if(p)
					stm << HTMLText(p);
			}
		}
/*
		Zifstream sFile(stgTmp.chars(), ios::nocreate|ios::in);
		if(sFile && sFile.good())
		{
			// Read the signature file
			while(sFile.NextLine())
			{
				if(sFile.eof() != 0)
					break;
				p = sFile.GetLine();
				stm << HTMLText(p) << endl;
			}
		}
*/

		stm << "</textarea>" << endl
			<< "</form>" << endl
#ifdef SPELLCHECK
			<< "<form name=SpellCheckForm method=POST action="
			<< NEWSBIN << "/spellchk target=SpellCheck>" << endl
			<< "<input type=hidden name=Body value=\"\">" << endl
			<< "<input type=hidden name=FormName value=\"\">" << endl
			<< "<input type=hidden name=FieldName value=\"\">" << endl
			<< "</form>" << endl
#endif
			<< "</font><p>" << endl
		;

		delete partspool;
	}

	post.OutEnd(stm, stgGroup, tmplFile);
	return stm;
}

void NewsPost::OutToolbar(ostream& stm, const char * pGroup,
	const char * pMessageID)
{
	ZString stgGroup = pGroup;

#ifdef SPELLCHECK
	stm
		<< "<script language=javascript>" << endl
		<< "<!--" << endl
		<< "function SpellCheck()" << endl
		<< "{" << endl
//		<< "  if(document.PostForm.TypeHTML.checked)" << endl
//		<< "  {" << endl
//		<< "    alert(\"The Spell Check funcion is not available for HTML "
//		<< "Content-Type.\\nPlease uncheck the \\\"Content-Type\\\" to perform "
//		<< "the spell check.\");" << endl
//		<< "  }" << endl
//		<< "  else" << endl
//		<< "  {" << endl
		<< "    var bText = document.PostForm.Body.value;" << endl
		<< "    if(bText.length == 0)" << endl
		<< "    {" << endl
		<< "      alert(\"No message for the spell check.\");" << endl
		<< "    }" << endl
		<< "    else" << endl
		<< "    {" << endl
		<< "      document.SpellCheckForm.FormName.value = \"PostForm\";"
		<< endl
		<< "      document.SpellCheckForm.FieldName.value = \"Body\";" << endl
		<< "      document.SpellCheckForm.Body.value = bText;" << endl
		<< "      var SpellWin = window.open(\"/spellwait.htm\", "
		<< "\"SpellCheck\", \"resizable=yes,scrollbars=yes,status=0,width=580,"
		<< "height=480\");" << endl
		<< "      document.SpellCheckForm.submit();" << endl
		<< "      if(navigator.appName=='Netscape')" << endl
		<< "      {" << endl
		<< "        SpellWin.focus();" << endl
		<< "      }" << endl
		<< "    }" << endl
#		<< "  }" << endl
		<< "}" << endl
		<< "// -->" << endl
		<< "</script>" << endl
	;
#endif
}

void NewsPost::OutEnd (ostream& stm, ZString &, Zifstream& tmplFile)
{
	stm << "<p>" << DRN_NOTICE << "</p>" << endl;

	char * p;
	while(tmplFile.NextLine())
	{
		p = tmplFile.GetLine();
		stm << p << endl;
	}
}
