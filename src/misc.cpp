#include <sys/stat.h>
#include <unistd.h>
#include <string>

#include "def.h"

//
//  Try to make one directory.
//
BOOL
MakeDir(const char * pName)
{
	struct stat Sb;

	if (mkdir(pName, 0775) >= 0)
	{
		chmod(pName, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
		return TRUE;
	}

	// See if it failed because it already exists.
	return stat(pName, &Sb) >= 0 && S_ISDIR(Sb.st_mode);
}

//
//  Given a directory, comp/foo/bar, create that directory and all
//  intermediate directories needed.
//
BOOL
MakeDirectory (const char * dir)
{
    	// Optimize common case -- parent almost always exists.
	if (MakeDir(dir))
		return TRUE;

	// Try to make each of comp and comp/foo in turn.
	string szName;
	const char * p = dir;
	int nLen = strlen(dir);
	for(int i = 0; i < nLen; i++)
	{
		if (p[i] == '/')
		{
			szName = string(p, i);
			if(!MakeDir(szName.c_str()))
				return FALSE;
		}
	}

	return MakeDir(dir);
}

//
BOOL
MakeOVDirectory (const char * p)
{
		if(chdir(NEWSINDEX) != 0)
		{
			DMSG(0, "Cannot chdir to %s", NEWSINDEX);
			return FALSE;
		}

		if(!MakeDirectory(p))
		{
			DMSG(0, "Cannot mkdir %s", p);
			return FALSE;
		}

		umask(0);
		return TRUE;
}
