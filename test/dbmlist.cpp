#include <unistd.h>
#include <string.h>
#include <builtin.h>
#include <ndbm.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "def.h"

//
// Usage:: dbmlist dbname [key}
//
main(int argc, char *argv[])
{
	if(argc < 2 || argc > 3)
	{
		cout << "Usage: dbmlist dbname [key]" << endl;
		exit(1);
	}

	char * pName = argv[1];
	mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
	DBM * pDB = dbm_open(pName, O_RDWR, mode);
  	if(pDB == NULL)
		pDB = dbm_open(pName, O_RDWR|O_CREAT|O_TRUNC, mode);
	if(pDB == NULL)
	{
		cout << "Cannot open database file " << pName << endl;
		exit(1);
	}

	datum key, resp;

	if(argc > 2)
	{
		// delete key
		key.dptr = argv[2];
		key.dsize = strlen(argv[2]);
		dbm_delete(pDB, key);
		cout << "Delete key:" << key.dptr << endl;
	}
	else
	{
		for(key = dbm_firstkey(pDB); key.dptr != NULL; key=dbm_nextkey(pDB))
		{
			resp = dbm_fetch(pDB, key);
			cout.form("%*.*s : ", key.dsize, key.dsize, key.dptr);
			if(resp.dptr)
				cout.form("(%d) %*.*s\n", resp.dsize, resp.dsize,
					resp.dsize, resp.dptr);
//				cout.form("%*.*s\n", resp.dsize, resp.dsize, resp.dptr);
			else
				cout << "empty" << endl;
		}
	}

	dbm_close(pDB);
}
