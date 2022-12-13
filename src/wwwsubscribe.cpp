#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "def.h"
#include "zcgi.h"
#include "userinfo.h"
#include "tmplerr.h"
#include "newssub.h"

extern const char szVersion[];
extern const char szBuild[];

UserInfo gUserInfo;

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

	BOOL bClear = FALSE;
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
			bClear = TRUE;
		av++;
		ac--;
	}

	ZCGI * pCGI = NULL;;
	if (strcasecmp(getenv("REQUEST_METHOD"), "POST") == 0)
	{
		pCGI = new ZCGI();
		if(!pCGI->ReadInput())
		{
			tmplerr.OutError(cout, "<h2>Internal error...</h2>");
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
		ZString filename = USERDIR;
		filename += gUserInfo.GetUserName();
		filename += "/sublist";
		BOOL bPrintMsg = TRUE;
		BOOL bFoundGroup = FALSE;
		const char * p;
		if(bClear)
		{
			NewsGroupSubscribe sub(filename.chars());

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
						 << "> <a href=\"" << NEWSBIN << "/wwwnews?"
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
			ZString tmpfile = USERDIR;
			tmpfile += gUserInfo.GetUserName();
			struct stat sb;
			if(stat(tmpfile.chars(), &sb) == -1)
			{
				if(errno == ENOENT)
				{
					DMSG(1, "create dir %s", tmpfile.chars());
					if(mkdir(tmpfile.chars(), 0755) == 0)
						chown(tmpfile, getuid(), getgid());
				}
			}

			char buf[16];
			sprintf(buf, "%05d", getpid());
			tmpfile += "/sub";
			tmpfile += buf;
			FILE * file = fopen(tmpfile.chars(), "w");
			if(!file)
			{
				cout << "<h2>Cannot save the newsgroups you have just "
					 << "subscribed. Please report the problem to the "
					 << "technical support</h2>"
					 << endl;
			}
			else
			{
				chown(tmpfile, getuid(), getgid());
				for(ParamMap::iterator iter = pCGI->GetMap().begin();
					iter != pCGI->GetMap().end(); iter++)
				{
					if((p = (*iter).first.c_str()) != NULL && *p)
					{
						if(strcmp(p, "submit.x") != 0 &&
							strcmp(p, "submit.y") != 0)
						{
							if(bPrintMsg)
							{
								cout << "<strong>Changes will not be "
									 << "recorded until the &quot;Save "
									 << "Settings&quot; button at the end of"
									 << "<br>" << endl
									 << "the page is clicked.</strong><p>"
									 << endl
									 << "<form action=\"" << NEWSBIN
									 << "/wwwsubscribe\" "
									 << "method=POST>" << endl;
					
								bPrintMsg = FALSE;
							}
							cout << "<input type=checkbox name="
								 << p << " CHECKED> <a href="
								 << NEWSBIN << "/wwwnews?" << URLText(p)
								 << ">" << HTMLText(p) << "</a><br>" << endl;
							fputs(p, file);
							fputc('\n', file);
							bFoundGroup = TRUE;
						}
					}
				}
				fclose(file);

				if(!bFoundGroup)
				{
					cout << "</form><p>" << endl
						 << "<h2>All the newsgroups are unsubscribed.</h2>"
						 << endl;
					remove(filename.chars());
				}
				else
				{
					cout << "<p>" << endl
						 << "<input src=/drn/images/prefsave.gif name=submit "
						 << "border=0 type=image alt=\"Submit Subscribe "
						 << "Newsgroups\">" << endl
						 << "<p>" << endl
						 << "</form><p>" << endl;

					DMSG(1, "rename %s to %s", tmpfile.chars(),
						filename.chars());
					if(rename(tmpfile.chars(), filename.chars()) == -1)
					{
						DMSG(1, "rename error: %s", strerror(errno));
						cout << "<h2>Cannot save your new subscribe "
							 << "group list, please contact the "
							 << "technical support.</h2>" << endl;
					}
				}
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
