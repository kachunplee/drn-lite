#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "def.h"
#include "zcgi.h"
#include "userinfo.h"
#include "tmplerr.h"
#include "newsgrp.h"
#include "newssub.h"

extern const char szVersion[];
extern const char szBuild[];

UserInfo gUserInfo;

static void OutGroup(NewsGroupSubscribe &, const char *); 
static BOOL SetSibs(ZString &, const char *, int); 
static void OutSibs(const char *, int); 

main(int ac, char ** av)
{
	NewsOption options;

	TemplateError tmplerr;
	ZString sErr;

	cout << "Content-Type: text/html" << endl << endl;

	if(!gUserInfo.Init(cout, getenv("REMOTE_USER"),
		getenv("REMOTE_HOST"), getenv("REMOTE_ADDR")))
	{
		tmplerr.OutError(cout,
				"<h3>User preference is not available in this server</h3>");
		return(1);
	}

	options.ReadPreference();

	BOOL bSubscribe = FALSE;
	BOOL bCheck = TRUE;
	int nLevel = 0;
	ZString grpList = "*";
	char * q;
	while(ac > 1)
	{
		if(strcmp(av[1], "-v") == 0)
		{
			cout << DEF_BODY_TAG << endl
				 << "<h3>DRN " << szVersion << szBuild
				 << " (c) 1995-1999 Pathlink Technology</h3>" << endl;
			return(1);
		}
		else if(strcmp(av[1], "-D") == 0)
			debug++;
		else if(strcmp(av[1], "-c") == 0)
			// reset all the checked groups
			bCheck = FALSE;
		else if(strncmp(av[1], "-g", 2) == 0)
		{
			// collapse level
			q = av[1] + 2;
			if(isdigit(*q))
				nLevel = atoi(q);
		}
		else if(strcmp(av[1], "-s") == 0)
			// Display subscribe list only
			bSubscribe = TRUE;
		else
		{
			strstrip(av[1], '\\');
			char * r = av[1];
			char s[strlen(r)+1];
			q = s;
			while(*r)
			{
				if(*r ==  '%')
				{
					if(*(r+1) && *(r+2))
					{
						// convert pair of hex char to integer
						*q++ = (hex(*(r+1))<<4) + hex(*(r+2));
						r += 3;
						continue;
					}
				}
				*q++ = *r++;
			}
			*q = '\0';
			grpList = s;
		}
		av++;
		ac--;
	}

	if (strcasecmp(getenv("REQUEST_METHOD"), "POST") == 0)
	{
		ZCGI cgi;
		if(!cgi.ReadInput())
		{
			tmplerr.OutError(cout, "<h2>Internal error...</h2>");
			return(1);
		}
		grpList = cgi.GetParam("newsgroups");
		if(grpList.length() < 1)
		{
			tmplerr.OutError(cout, "<h2>Missing Data for the DRN Newsgroup Subscription. Please report the problem to technical support.</h2>");
			return(1);
		}
	}

	ZString TmplName = DRNTMPLDIR;
	TmplName += "/listgroup.htm";
	Zifstream tmplFile(TmplName, ios::nocreate|ios::in);
	if(!tmplFile)
	{
		ZString err = "<h3>Template file " + TmplName;
		err += " is not found</h3>";
		tmplerr.OutError(cout, err.chars());
		return(1);
	}

	if(!tmplFile.good())
	{
		ZString err = "<h3>Template file " + TmplName;
		err += " is empty</h3>";
		tmplerr.OutError(cout, err.chars());
		return(1);
	}

	// Scan for <!--pathlink drn=listgroup -->
	BOOL bDspPost = FALSE;
	do
	{
		q = tmplFile.GetLine();

		if(strncmp(q, "<!--pathlink drn=listgroup -->", 30) == 0)
		{
			bDspPost = TRUE;
			break;
		}

		cout << q << endl;
	} while(tmplFile.NextLine());

	if(bDspPost)
	{
		//
		// Fix up search Pattern
		//
		// Throw away trailling '|' (OR)
		//
		NewsGroupList list(grpList, options);

		ListNewsGroup * lst = NULL;
		grpList = "^" + grpList;
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
					lst = new ListNewsGroupG(&list, TRUE);
				}
				else
					lst = new ListNewsGroupG(&list);
				grpList += '&';
			}
		}
		else
		{
			while(grpList.lastchar() == '|')
				grpList.DelLast();
		}

		if(lst == NULL)
			lst = new ListNewsGroupL(&list);

		if(!lst->open())
		{
			tmplerr.OutError(cout, "<h3>Cannot open Group List file</h3>");
			delete lst;
			return(1);
		}

		grpList.downcase();
		grpList.gsub(".", "\\.");
		grpList.gsub("+", "\\+");
		grpList.gsub('?', '.');
		grpList.gsub("*", ".*");
		grpList.gsub("&", "[^.]*");

		ZString filename = USERDIR;
		filename += gUserInfo.GetUserName();
		filename += "/sublist";
		NewsGroupSubscribe sub(filename.chars());

		BOOL bPrintMsg = TRUE;
		BOOL bFoundGroup = FALSE;
		BOOL bFoundSibs = FALSE;
		const char * p;
		if(bSubscribe)
		{
			for(GrpSubMap::iterator iter = sub.GetMap().begin();
				iter != sub.GetMap().end(); iter++)
			{
				if(bPrintMsg)
				{
					cout << "<strong>Changes will not be recorded until the "
						 << "&quot;Save Settings&quot; button at the end of"
						 << "<br>" << endl
						 << "the page is clicked.</strong><p>" << endl
						 << "<form action=\"" << NEWSBIN << "/wwwsubscribe\" "
						 << "method=POST>" << endl;
					
					bPrintMsg = FALSE;

				}
				if((p = (*iter).first.c_str()) != NULL && *p)
				{
					cout << "<input type=checkbox name=" << p
						 << " CHECKED> <a href=\"" << NEWSBIN << "/wwwnews?"
						 << URLText(p) << "\">" << HTMLText(p) << "</a><br>"
						 << endl;
				}
				bFoundGroup = TRUE;
			}

			if(!bFoundGroup)
			{
				cout << "<h2>You have not subscribed to any newsgroup.</h2>"
					 << endl;
			}
			else
			{
				cout << "<p>" << endl
					 << "<input src=/drn/images/prefsave.gif name=submit "
					 << "border=0 type=image alt=\"Submit Subscribe "
					 << "Newsgroups\">" << endl
					 << "<p>" << endl
					 << "</form><p>" << endl;
			}
		}
		else
		{
			Regex ex(grpList.chars());
			int nArt;
			BOOL bModerate;
			ZString stgGroup = "";
			ZString stgSibs = "";
			int nGroup = 0;
			int nLen = 0;
			int maxLevel;
			while((p = lst->GetLine(ex, &nArt, &bModerate)))
			{
				if(bPrintMsg)
				{
					cout << "<strong>Changes will not be recorded until the "
						 << "&quot;Save Settings&quot; button at the end of"
						 << "<br>" << endl
						 << "the page is clicked.</strong><p>" << endl
						 << "<form action=\"" << NEWSBIN << "/wwwsubscribe\" "
						 << "method=POST>" << endl;
						
					bPrintMsg = FALSE;

				}
				if(strncmp(p, "alt.", 4) == 0)
					maxLevel = 4;
				else
					maxLevel = 3;
				if(nLevel > 0 && nLevel < maxLevel)
				{
					if(nLen > 0 && strncmp(stgSibs.chars(), p, nLen) == 0)
						nGroup++;
					else
					{
						if(nGroup > 0)
						{
							if(nGroup == 1)
							{
								OutGroup(sub, stgGroup.chars());
								bFoundGroup = TRUE;	
							}
							else
							{
								stgSibs.replace(nLen-1, 1, '*');
								OutSibs(stgSibs.chars(), nLevel);
								bFoundSibs = TRUE;	
							}
							nGroup = 0;
						}
						nLen = 0;
						if(SetSibs(stgSibs, p, nLevel))
						{
							nGroup = 1;
							nLen = strlen(stgSibs);
						}
						else
						{
							OutGroup(sub, p);
							bFoundGroup = TRUE;	
						}
					}
				}
				else
				{
					OutGroup(sub, p);
					bFoundGroup = TRUE;
				}
				stgGroup = p;
			}

			if(nGroup == 1)
			{
				OutGroup(sub, stgGroup.chars());
				bFoundGroup = TRUE;	
			}
			else if(nGroup > 1)
			{
				stgSibs.replace(nLen-1, 1, '*');
				OutSibs(stgSibs.chars(), nLevel);
				bFoundSibs = TRUE;	
			}

			if(!bFoundGroup && !bFoundSibs)
			{
				cout << "<h2>Cannot find any matching newsgroup.</h2>" << endl;
			}
			else if(bFoundGroup)
			{
				for(GrpSubMap::iterator iter = sub.GetMap().begin();
					iter != sub.GetMap().end(); iter++)
				{
					if((*iter).second == 0)
					{
						if((p = (*iter).first.c_str()) != NULL && *p)
							cout << "<input type=hidden name="
								 << (*iter).first.c_str()
								 << " value=on>" << endl;
					}
				}

				cout << "<p>" << endl
					 << "<input src=/drn/images/prefsave.gif name=submit "
					 << "border=0 type=image alt=\"Submit Subscribe "
					 << "Newsgroups\">" << endl
					 << "<p>" << endl
					 << "</form><p>" << endl;
			}
			else
			{
				cout << "</form><p>" << endl;
			}
		}
	}

	while(tmplFile.NextLine())
	{
		q = tmplFile.GetLine();
		cout << q << endl;
	}
	return(0);
}

static void
OutGroup(NewsGroupSubscribe & sub, const char * p) 
{
	cout << "<input type=checkbox name=" << p;
	if(sub.Find(p))
	{
		cout << " CHECKED";
		sub.Set(p, 1);
	}
	cout << "> <a href=\"" << NEWSBIN << "/wwwnews?" << URLText(p)
		 << "\">" << HTMLText(p) << "</a><br>" << endl;
}

static BOOL
SetSibs(ZString &stgSibs, const char *p, int nLevel)
{
	const char * q = p;
	int i;
	for(i = 0; i < nLevel; i++)
	{
		q = strchr(q, '.');
		if(q == NULL)
		{
			if(i == (nLevel-1))
			{
				stgSibs = p;
				stgSibs += ".";
				return TRUE;
			}
			stgSibs = "";
			return FALSE;
		}
		q++;
	}
	if(q)
	{
		stgSibs.assign(p, q-p);
		return TRUE;
	}
	stgSibs = "";
	return FALSE;
}

static void
OutSibs (const char * pSibs, int nLevel)
{
	cout << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"" << NEWSBIN
		 << "/wwwlistgrp?";
	int maxLevel;
	if(strncmp(pSibs , "alt.", 3) == 0)
		maxLevel = 4;
	else
		maxLevel = 3;
	if(nLevel < maxLevel)
		cout << "-g" << nLevel+1 << "+";
	cout << URLText(pSibs) << "\"><b>" << HTMLText(pSibs) << "</b></a><br>"
		 << endl;
}
