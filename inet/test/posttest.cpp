#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream.h>

#include "dmsg.h"
#include "cnntp.h"

extern int debug;

const char * NNTPservers = "clark.pathlink.com:1119";
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
			cout << "usage: posttest [-S servers] [-A authinfo] [-d]\n";
			return(1);
		}
	}

	ac -= optind;
	av += optind;

	const int MAXLINE=1024;
	char buf[MAXLINE+2];		// +2 = + \r + \0
	char * p;
	buf[0] = '.';
	try
	{
		CNNTP nntp(NNTPservers, NNTPauths);

		p = nntp.SendCmd("POST\r\n", "340 ");
		DMSG(1, "%s", p);

		for(;;)
		{
			// Getline into buf
			cin.getline(buf+1, MAXLINE-1);
			if(cin.eof()) break;
			if(!cin.good())
			{
				cerr << "Error: " << strerror(errno) << endl;
				return(1);
			}
			//
			int n = cin.gcount();
			p = buf+1;
			if(buf[1] == '.')
			{
				p--;
				n++;
			}

			strcpy(p+n-1, "\r\n");
			nntp.Send(p, n+1);
		}

		p = nntp.SendCmd(".\r\n", "240 ");
		DMSG(1, "%s", p);
	}
	catch(const char * p)
	{
		// 502 Authentication error
		//
		// 441 Article has no body -- just headers
		//
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
