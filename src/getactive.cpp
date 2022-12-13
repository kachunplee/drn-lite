#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "def.h"
#include "nntpact.h"

main(int ac, char ** av)
{
	extern char *optarg;
	extern int optind;

	int c;
	while((c = getopt(ac, av, "d")) != EOF)
	{
		switch(c)
		{
		case 'd':
			debug++;
			break;

		default:
		//usage:
			cout << "usage: getactive [-d]\n";
			return(1);
		}
	}

	av += optind;

	char * p;
	try
	{
		NNTPActive art;

		while((p=art.GetLine()))
		{
			cout << p << endl;
			if(p[0] == '.' && p[1] == '\r')
				break;
		}
	}
	catch(const char * p)
	{
		DMSG(1, "getactive: %s", p);
		return(1);
	}
	catch(int err)
	{
		DMSG(0, "getactive: &s", strerror(errno));
		return(1);
	}

	return(0);
}
