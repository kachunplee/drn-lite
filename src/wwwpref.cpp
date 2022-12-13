#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "def.h"
#include "advert.h"
#include "userinfo.h"
#include "tmplerr.h"
#include "newspost.h"

UserInfo gUserInfo;

main(int argc, char *argv[])
{
	NewsOption options;

	TemplateError tmplerr;
	ZString sErr;

	if(!gUserInfo.Init(cout, getenv("REMOTE_USER"),
		getenv("REMOTE_HOST"), getenv("REMOTE_ADDR")))
	{
		tmplerr.OutContentType();
		tmplerr.OutError(cout,
				"<h3>User preference is not available in this server</h3>");
		return(1);
	}

	options.ReadPreference();

	BOOL bUpdPref = FALSE;
	BOOL bUpdQuote = FALSE;
	BOOL bUpdSig = FALSE;

	if(argc > 1)
	{
		if(strcmp(argv[1], "-v") == 0)
		{
			cout << "Content-Type: text/html" << endl << endl
				 << DEF_BODY_TAG << endl
				 << "<h3>DRN " << szVersion << szBuild
				 << " (c) 1995-1999 Pathlink Technology</h3>" << endl;
			return(1);
		}

		while(argc > 1)
		{
			if(strcmp(argv[1], "-D") == 0)
				++debug;
			else if(strcmp(argv[1], "-p") == 0)
				bUpdPref = TRUE;
			else if(strcmp(argv[1], "-q") == 0)
				bUpdQuote = TRUE;
			else if(strcmp(argv[1], "-r") == 0)
				bUpdSig = TRUE;
			--argc;
			++argv;
		}
	}

	PCGI pCGI = NULL;
	if (strcasecmp(getenv("REQUEST_METHOD"), "POST") == 0)
	{
		pCGI = cgi_Create();
		if (!pCGI)
		{
			tmplerr.OutError(cout, "<h2>User Preference - Internal Error</h2>");
			return(1);
		}
		if (cgi_GetInput(pCGI) == 0)
		{
			tmplerr.OutError(cout, "<h2>Missing preference input</h2>");
			return(1);
		}
	}

	Preference pref(options, pCGI, bUpdPref, bUpdQuote, bUpdSig);
	cout << pref;
	return(0);
}
