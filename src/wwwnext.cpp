#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "def.h"
#include "advert.h"
#include "userinfo.h"
#include "newsmidx.h"
#include "tmplerr.h"

UserInfo gUserInfo;
const char GroupErrorMsg[] = "<h2>Please specify newsgroups article</h2>";

static void
OutError(ostream & stm, const char * p)
{
	TemplateError tmplerr;
	tmplerr.OutContentType(stm);
	tmplerr.OutError(stm, p);
}

main(int argc, char *argv[])
{
	NewsOption options;

	ZString sErr;
	if(!gUserInfo.Init(cout, getenv("REMOTE_USER"),
		getenv("REMOTE_HOST"), getenv("REMOTE_ADDR")))
	{
		sErr = "<h3>Please contact your ISP for DRN access information from this host: ";
		sErr += gUserInfo.GetHostName();
		sErr += "</h3>";
		OutError(cout, sErr.chars());
		return(1);
	}

	options.ReadPreference();

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
			OutError(cout, GroupErrorMsg);
			return(1);
		}
	
		//
		// Call from CGI
		//
		if(stgNewsgroup.contains("newsgroups=") == 0)
		{
			OutError(cout, "<h2>Please specify newsgroups=which_group</h2>");
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
		OutError(cout, GroupErrorMsg);
		return(0);
	}

	//
	// Look for article ID
	//
	if(stgNewsgroup[0] == '<' && stgNewsgroup.lastchar() == '>')
	{
		OutError(cout, GroupErrorMsg);
		return(0);
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
			ZString szGroup = stgNewsgroup;
			while(isdigit(szGroup.lastchar()))
				szGroup.DelLast();
			szGroup.DelLast();
			if(szGroup.length() == 0)
			{
				OutError(cout, GroupErrorMsg);
				return(0);
			}
			int nArt = GetArtNum(szGroup, NULL, atoi(pArt), TRUE,
				SORT_THREAD, 0, options.IsBinCol());
			if(nArt == -1)
			{
				URLText szURL(szGroup);
				URLText szHTML(szGroup);
				ZString sErr;
				sErr = "<h3><img src=/drn/images/drnheir.gif><a href=\"http:";
				sErr += NEWSBIN;
				sErr += "/wwwnews?";
				sErr += szURL.GetText();
				sErr += "\">";
				sErr += szHTML.GetText();
				sErr += "</a></h3>\n";
				sErr += "<h3>No more article</h3>";
				OutError(cout, sErr.chars());
			}
			else
			{
				char buf[24];
				sprintf(buf, "%d", nArt);
				stgNewsgroup = szGroup + "/";
				stgNewsgroup += buf;
				cout << "Location: " << DRN_SERVER
					<< NEWSBIN << "/wwwnews?" << options
					<< URLText(stgNewsgroup) << endl << endl;
			}
			return(0);
		}
	}

	OutError(cout, GroupErrorMsg);
	return(0);
}
