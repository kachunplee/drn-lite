#include <g++/iostream.h>
#include <g++/fstream.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/mount.h>
#include "drncache.h"

#define DECODEFS "/n0"
/*
#define DECODEFS "/var"
*/
#define DRNCLEANCACHE "/var/local/www/bin/cleancache"

main ()
{
	struct statfs sfs;
	if(statfs(DECODEFS, &sfs) < 0)
	{
		cout << "File system " << DECODEFS << " stat error\n";
		exit(0);
	}

	if(((sfs.f_bavail * 100) / sfs.f_blocks) < 10)
/*
	if(((sfs.f_bavail * 100) / sfs.f_blocks) < 55)
*/
	{
		cout << "Call " << DRNCLEANCACHE << "\n";
		system(DRNCLEANCACHE);
	}
}
