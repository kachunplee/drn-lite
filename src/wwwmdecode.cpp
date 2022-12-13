#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "def.h"
#include "cgi.h"
#include "userinfo.h"
#include "newsdecode.h"

UserInfo gUserInfo;

extern const char szVersion[], szBuild[];

void html_header (ostream& stm, char* pTitle)
{
	stm << "Content-Type: text/html" << endl << endl
		<< "<HTML><HEAD>" << endl;
	if (pTitle == NULL)
		stm << "<TITLE>" << pTitle << "</TITLE>" << endl;
	stm << "</HEAD><BODY>" << endl
		<< DEF_BODY_TAG << endl;
	if (pTitle == NULL)
	{
		stm << "<center><h2>" << pTitle << "</h2></center>" << endl
			<< "<hr><p>" << endl;
	}
}

void html_trailer (ostream& stm)
{
	stm << "<p>" << endl
		<< "</BODY></HTML>" << endl;
}

void display_errmsg (ostream& stm)
{
	html_header(stm, NULL);
	stm << "<h2>Decode internal error</h2>" << endl;
	html_trailer(stm);
}

int main (int argc, char *argv[])
{
	NewsOption options;

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

	setpriority(PRIO_PROCESS, 0, 10);
	if (strcasecmp(getenv("REQUEST_METHOD"), "POST"))
	{
		html_header(cout, "Input format error...");
		cout << "<h2>Decode internal error</h2>" << endl;
		html_trailer(cout);
		return(-1);
	}

	if(!gUserInfo.Init(cout, getenv("REMOTE_USER"),
		getenv("REMOTE_HOST"), getenv("REMOTE_ADDR")))
	{
		html_header(cout, "");
		cout << NO_GROUP_MSG << endl;
		html_trailer(cout);
		return(1);
	}

	options.ReadPreference();

	PCGI pCGI = cgi_Create();
	if (!pCGI)
	{
		display_errmsg(cout);
		return(-1);
	}
	if (cgi_GetInput(pCGI) == 0)
	{
		return(-1);
	}
	ZString FileName = cgi_GetValueByName(pCGI, "stgArticle");
	int nLen = FileName.length();
	if (!nLen)
		display_errmsg(cout);
	NewsDecode* pDecode = new NewsDecode(FileName, -2, FALSE);
	if (!pDecode)
	{
		display_errmsg(cout);
		cgi_Destroy(pCGI);
		return(-1);
	}
	if(!pDecode->InitMultiFileDecode(pCGI))
	{
		display_errmsg(cout);
		delete pDecode;
		cgi_Destroy(pCGI);
		return(-1);
	}

	int nMulti = pDecode->GetMulti();
	if (nMulti == MULTI_MIME)
	{
		pDecode->SetMulti(MULTI_NONE);
		cout << *pDecode;
	}
	else if (nMulti == MULTI_UUE)
		cout << *pDecode;
	else
		display_errmsg(cout);

	delete pDecode;
	cgi_Destroy(pCGI);
	return(0);
}
