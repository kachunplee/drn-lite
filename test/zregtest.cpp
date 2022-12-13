#include "zstg.h"

main ()
{
	ZString istg;
	ZString estg;
	const char * p;
	Regex * pex = NULL;
	regmatch_t match;
	int result;

	while(readline(cin, istg) > 0)
	{
		p = istg.chars();

		switch(p[0])
		{
		case '+':
			estg = p+1;
			delete pex;
			pex = new Regex(estg);
			continue;
		}

		if(pex == NULL)
		{
			cout << "Please set Regex by entering +..." << endl;
			continue;
		}

		if(p[0] == '-') p++;
		result = pex->RegExec(p, 1, &match);
		cout << "Result = " << result << endl;
		if(result == 0)
			cout << "  " << match.rm_so << "-" << match.rm_eo << endl;
	}
}
