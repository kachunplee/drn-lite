#include "def.h"

#include "newslartn.h"

const char * NewsListArtNum::GetFileName () { return "/article.i"; }

int NewsListArtNum::GetOverField ()
{
	return 0;
}

compar NewsListArtNum::GetCompar ()
{
	return NULL;
}
