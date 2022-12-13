#include <unistd.h>
#include <string.h>

#include "def.h"
#include "newsgrp.h"
#include "newslist.h"
#include "newsart.h"

UserInfo gUserInfo;
BOOL bLynx = FALSE;

main(int argc, char *argv[])
{
	ZString agent = getenv("HTTP_USER_AGENT");
	agent.downcase();
	if(agent.contains(LYNX))
		bLynx = TRUE;

	if(!gUserInfo.Init(cout, getenv("REMOTE_USER"),
		getenv("REMOTE_HOST"), getenv("REMOTE_ADDR")))
	{
		cout << "Content-Type: text/html" << endl << endl
			<< DEF_BODY_TAG << endl;

		cout << "<h3>Please contact your ISP for DRN access information from this host: "
			<< gUserInfo.GetHostName() << "</h3>" << endl;
		exit(1);
	}

	cout << "Content-Type: text/html" << endl << endl
		<< DEF_BODY_TAG << endl;

	NewsOption options;
	ZString stgNewsgroup;
	if(argc > 1)
	{
		//
		// Use arguments
		//
		if(--argc > 1 && argv[1][0] == '-')
		{
			options.SetOptions(++argv[1]);
		}

		//
		// strip '\\'
		//
		strstrip(argv[argc], '\\');
		char * p = argv[argc];
		char s[strlen(p)+1];
		char * q = s;
		while(*p)
		{
			if(*p ==  '%')
			{
				if(*(p+1) && *(p+2))
				{
					// convert pair of hex char to integer
					*q++ = (hex(*(p+1))<<4) + hex(*(p+2));
					p += 3;
					continue;
				}
			}
			*q++ = *p++;
		}
		*q = '\0';
		stgNewsgroup = s;
	}
	else
	{
		stgNewsgroup = getenv("QUERY_STRING");
		if(stgNewsgroup.length() == 0)
		{
			cout << "<h2>Please specify newsgroups pattern | newsgroup | article</h2>" << endl;
			if(gUserInfo.IsConnect())
				gUserInfo.AdjConnect();
			exit(1);
		}
	
		//
		// Call from CGI
		//
		if(stgNewsgroup.contains("newsgroups=") == 0)
		{
			cout << "<h2>Please specify newsgroups=which_group</h2>" << endl;
			if(gUserInfo.IsConnect())
				gUserInfo.AdjConnect();
			exit(1);
		}
		stgNewsgroup = stgNewsgroup.after('=');
		const char * p = stgNewsgroup.chars();
		char s[stgNewsgroup.length()+1];
		char * q = s;
		while(*p && *p != '&')
		{
			switch(*p)
			{
			case '+':
				*q++ = ' ';
				p++;
				continue;

			case '%':
				if(*(p+1) && *(p+2))
				{
					// convert pair of hex char to integer
					*q++ = (hex(*(p+1))<<4) + hex(*(p+2));
					p += 3;
					continue;
				}
			}
			*q++ = *p++;
		}
		*q = '\0';
		stgNewsgroup = s;
	}

	if(stgNewsgroup.length() == 0)
	{
		cout << "<h2>Please specify newsgroups pattern | newsgroup | article</h2>" << endl;
		if(gUserInfo.IsConnect())
			gUserInfo.AdjConnect();
		exit(0);
	}

	//
	// Look for article ID
	//
	if(stgNewsgroup[0] == '<' && stgNewsgroup.lastchar() == '>')
	{
		NewsArticle art(stgNewsgroup, options, -1);
		cout << art;
		if(gUserInfo.IsConnect())
			gUserInfo.AdjConnect();
		exit(0);
	}

	//
	// Look for article Number
	//
	const char * p = stgNewsgroup.chars();
	const char * q = strrchr(p, '/');
	const char * pArt = q;
	if(q)
	{
		while(*++q) { if(!isdigit(*q)) break; }
		while(*++pArt) { if(isdigit(*pArt)) break; }

		if(*q == 0)
		{
			NewsArticle art(stgNewsgroup, options, atoi(pArt));
			cout << art;
			if(gUserInfo.IsConnect())
				gUserInfo.AdjConnect();
			exit(0);
		}
	}

	//
	// stgNewsgroup a Newsgroup or Newsgroups pattern
	//
	if(stgNewsgroup.contains(Regex("[?&*|]")))
	{
		//
		// Got Wild cards characters -> group list
		//
		NewsGroupList list(stgNewsgroup, options);
		cout << list;
		if(gUserInfo.IsConnect())
			gUserInfo.AdjConnect();
		exit(0);
	}

	NewsArticleList list(stgNewsgroup, options);
	cout << list;
	if(gUserInfo.IsConnect())
		gUserInfo.AdjConnect();
	exit(0);
}
