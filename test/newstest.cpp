#include "def.h"

main (int argc, char ** argv)
{
	if(argc != 2)
	{
		cerr << "usage: newstest arg" << endl;
		exit(1);
	}

	static Regex expFileType("\\.jpg\\|\\.gif\\|\\.uue\\|\\.avi\\|\\.mov\\|\\.mpg", 1);
	String stg = argv[1];
	cout << stg.contains(expFileType) << endl;
}


//
void TestSubjMultiPart(char * p)
{
	NewsSubject subj(p);
	int n, l;
	int r =	subj.IsMultiPart(&n, &l);

	cerr << "IsMulti = " << r;
	if(r)
		cerr << " (" << n << "/" << l << ")";
	cerr << endl;
}
