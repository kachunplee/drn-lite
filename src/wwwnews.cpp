#include <unistd.h>
#include <string.h>

#include "def.h"
#include "userinfo.h"
#include "newsgrp.h"
#include "newslist.h"
#include "newsart.h"

UserInfo gUserInfo;

extern const char szVersion[], szBuild[];

main(int argc, char *argv[])
{
	NewsOption options;

	ZString tmpStg;

	if(!gUserInfo.Init(cout, getenv("REMOTE_USER"),
		getenv("REMOTE_HOST"), getenv("REMOTE_ADDR")))
	{
		cout << "Content-Type: text/html" << endl << endl
			 << DEF_BODY_TAG << endl;

		cout << "<h3>Please contact your ISP for DRN access information from this host: "
			<< gUserInfo.GetHostName() << "</h3>" << endl;
		return(1);
	}

	options.ReadPreference();

	cout << "Content-Type: text/html" << endl << endl;

	ZString stgNewsgroup;
	if(argc > 1)
	{
		if(strcmp(argv[1], "-v") == 0)
		{
			cout << DEF_BODY_TAG << endl
				 << "<h3>DRN " << szVersion << szBuild
				 << " (c) 1995-1999 Pathlink Technology" << endl;
			return(1);
		}

		while(argc > 1 && strcmp(argv[1], "-D") == 0)
		{
			++debug;
			--argc;
			++argv;
		}

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
			cout << DEF_BODY_TAG
				 << "<h2>Please specify newsgroups pattern | newsgroup | article</h2>" << endl;
			return(1);
		}
	
		//
		// Call from CGI
		//
		if(stgNewsgroup.contains("newsgroups=") == 0)
		{
			cout << DEF_BODY_TAG
			 	 << "<h2>Please specify newsgroups=which_group</h2>" << endl;
			return(1);
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
		cout << DEF_BODY_TAG << endl
			 << "<h2>Please specify newsgroups pattern | newsgroup | article</h2>" << endl;
		return(0);
	}

	//
	// Look for article ID
	//
	if(stgNewsgroup[0] == '<' && stgNewsgroup.lastchar() == '>')
	{
		NewsArticle art(stgNewsgroup, options, -1);
		cout << art;
		return(0);
	}

	//
	// Look for article Number
	//
	const char * p = stgNewsgroup.chars();
	const char * q = strrchr(p, '/');
	if(q)
	{
		const char * pArt = ++q;
		for(; *q; q++) { if(!isdigit(*q)) break; }
		if(*q == 0)
		{
			NewsArticle art(stgNewsgroup, options, pArt-stgNewsgroup.chars());
			cout << art;
			return(0);
		}
	}

	//
	// stgNewsgroup a Newsgroup or Newsgroups pattern
	//
	if(stgNewsgroup.containsSet("[?&*|]"))
	{
		//
		// Got Wild cards characters -> group list
		//
		NewsGroupList list(stgNewsgroup, options);
		cout << DEF_BODY_TAG << endl;
		cout << list;
		return(0);
	}

	NewsArticleList list(stgNewsgroup, options);
	cout << list;
	return(0);
}
