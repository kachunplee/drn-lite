#include <iostream.h>
#include <string>

#include "def.h"
#include "newslib.h"

main()
{
	int nPart, nTotal;

	ZString stg;
	while(readline(cin, stg) > 0)
	{
		cout << stg << endl;

		NewsSubject subj(stg.chars());
		if(subj.IsMultiPart(stg, &nPart, &nTotal))
		{
			cout << "Multi Part " << nPart << " of " << nTotal << endl;
			cout << stg << endl;
		}
		else
		{
			cout << "Not a multi part" << endl;
		}
	}
}
