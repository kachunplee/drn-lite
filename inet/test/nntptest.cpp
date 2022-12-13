#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream.h>

#include "cnntp.h"

extern int debug;

const char * NNTPservers = "dex.pathlink.com:1119";
const char * NNTPauths = "teresa11/111111";

main(int argc, char ** argv)
{
	debug = 1;

	if(argc > 1)
		NNTPservers = argv[1];

	if(argc > 2)
		NNTPauths = argv[2];

	char * p;
	try
	{
		CNNTP server(NNTPservers, NNTPauths);

		p = server.SendCmd("help\r\n");
		while((p=server.GetLine()))
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
