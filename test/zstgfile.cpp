#include <fstream.h>
#include "zstg.h"

main ()
{
	ZString stg;
	int n;

	ifstream stm("/var/local/drn/www/etc/drn.conf");
	while((n=readline(stm, stg)) != 0)
	{
		cout << n << ": " << stg << endl;
	}
}
