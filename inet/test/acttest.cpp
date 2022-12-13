#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream.h>

#include "nntpact.h"

extern int debug;

const char * NNTPservers = "lex.pathlink.com:1119";
const char * NNTPauths = "teresa11/111111";

main(int ac, char ** av)
{
	debug = 1;

	extern char *optarg;
	extern int optind;

	int c;
	while((c = getopt(ac, av, "S:A:d")) != EOF)
	{
		switch(c)
		{
		case 'S':
			NNTPservers = optarg;
			break;

		case 'A':
			NNTPauths = optarg;
			break;

		case 'd':
			debug++;
			break;

		default:
		//usage:
			cout << "usage: acttest [-S servers] [-A authinfo] [-d]\n";
			return(1);
		}
	}

	//ac -= optind;
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
		cerr << "Error: " << p << endl;
		return(1);
	}
	catch(int err)
	{
		cerr << "Error: " << strerror(errno) << endl;
		return(1);
	}

	return(0);
}
