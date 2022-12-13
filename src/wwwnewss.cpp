#include <unistd.h>
#include <string.h>

#include "def.h"
#include "userinfo.h"
#include "advert.h"

UserInfo gUserInfo;

main(int argc, char *argv[])
{
	cout << "Content-Type: text/html" << endl << endl <<
		DEF_BODY_TAG << endl;

	ZString stgNewsgroup("newsadm.test");
	if(argc > 1)
		stgNewsgroup = argv[1];
//	AdvertBanner banner(stgNewsgroup);
	cout
		<< "<title>Newsadm News Service - " << stgNewsgroup << "</title>"
//		<< endl
//		<< banner
		<< "<hr>" << endl
		;

	cout
		<< "T  "
		<< "<a href=" << NEWSBIN << "/wwnewss?"
		<< stgNewsgroup << ">"
		<< "&#80;&#108;&#101;&#97;&#115;&#101;&#32;"
		<< "&#100;&#105;&#115;&#97;&#98;&#108;&#101;&#32;&#109;&#101;"
		<< "</a>"
		<< endl
	;
	return(0);
}
