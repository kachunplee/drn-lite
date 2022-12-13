#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "def.h"
#include "advert.h"
#include "userinfo.h"
#include "tmplerr.h"
#include "newspost.h"

UserInfo gUserInfo;

extern const char szVersion[], szBuild[];

main(int argc, char *argv[])
{
	BOOL bAttach = TRUE;

	NewsOption options;

	TemplateError tmplerr;
	ZString sErr;

	cout << "Content-Type: text/html" << endl << endl;

	if(!gUserInfo.Init(cout, getenv("REMOTE_USER"),
		getenv("REMOTE_HOST"), getenv("REMOTE_ADDR")))
	{
		sErr = "<h3>Please contact your ISP for DRN access information from this host: ";
		sErr += gUserInfo.GetHostName();
		sErr += "</h3>";
		tmplerr.OutError(cout, sErr.chars());
		return(1);
	}

	options.ReadPreference();

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

		if(strcmp(argv[1], "-t") == 0)
		{
			bAttach = FALSE;
			--argc;
			++argv;
		}

		//
		// Use arguments
		//
		if(argc > 1 && argv[1][0] == '-')
		{
			--argc;
			options.SetOptions(++argv[1]);
		}
	}

	if(argc > 1)
	{
		//
		// strip '\\'
		//
		char * p = argv[argc-1];
		strstrip(p, '\\');
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
			tmplerr.OutError(cout, "<h2>Please specify newsgroups article</h2>");
			return(1);
		}
	
		//
		// Call from CGI
		//
		if(stgNewsgroup.contains("newsgroups=") == 0)
		{
			tmplerr.OutError(cout,
					"<h2>Please specify newsgroups=which_group</h2>");
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
		tmplerr.OutError(cout, "<h2>Please specify newsgroup article</h2>");
		return(0);
	}

	//
	// Look for article ID
	//
	if(stgNewsgroup[0] == '<' && stgNewsgroup.lastchar() == '>')
	{
		tmplerr.OutError(cout, "<h2>Please specify newsgroup article</h2>");
		return(0);
	}

	NewsPost post(stgNewsgroup, options, bAttach);
	cout << post;
	return(0);
}
