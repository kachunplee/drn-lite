#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "def.h"

#include "userinfo.h"
#include "tmplerr.h"
#include "newsdecode.h"

UserInfo gUserInfo;

extern const char szVersion[], szBuild[];

main(int argc, char *argv[])
{
	NewsOption options;

	TemplateError tmplerr;
	ZString sErr;

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

		while(argc > 1 && strcmp(argv[1], "-D") == 0)
		{
			++debug;
			--argc;
			++argv;
		}
	}

	if(argc != 2)
	{
		tmplerr.OutContentType(cout);
		tmplerr.OutError(cout, "<h2>Please specify article</h2>");
		return(1);
	}

	if(!gUserInfo.Init(cout, getenv("REMOTE_USER"),
		getenv("REMOTE_HOST"), getenv("REMOTE_ADDR")))
	{
		tmplerr.OutContentType(cout);
		tmplerr.OutError(cout, NO_GROUP_MSG);
		return(1);
	}

	options.ReadPreference();

	ZString stgNewsArt = argv[1];
	int nPart = -1;
	if(stgNewsArt.contains(':'))
	{
		ZString stg = stgNewsArt.after(':');
		nPart = atoi(stg.chars());
		stgNewsArt = stgNewsArt.before(':');
	}

	setpriority(PRIO_PROCESS, 0, 10);
	NewsDecode decode(stgNewsArt, nPart, TRUE);
	cout << decode;
}
