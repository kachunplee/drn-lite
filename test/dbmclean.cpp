#include <unistd.h>
#include <string.h>
#include <builtin.h>
#include <ndbm.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "def.h"

static ZString szFileName;
static ZString szMsgName;

void UnLockFile (char * pName, int * pfd)
{
	if(*pfd > -1 && *pName)
	{
		close(*pfd);
		ZString stgTemp = LOCKDIR;
		stgTemp += pName;
		remove(stgTemp);
		*pfd = -1;
	}
}

void LockFile (char * pName, int * pfd)
{
	if(*pfd > -1)
		return;

	if(!bSignal)
	{
		bSignal = TRUE;
		signal(SIGHUP, CleanUp);
		signal(SIGINT, CleanUp);
		signal(SIGPIPE, CleanUp);
		signal(SIGALRM, CleanUp);
		signal(SIGTERM, CleanUp);
		signal(SIGXCPU, CleanUp);
		signal(SIGXFSZ, CleanUp);
		signal(SIGVTALRM, CleanUp);
		signal(SIGPROF, CleanUp);
		signal(SIGUSR1, CleanUp);
		signal(SIGUSR2, CleanUp);
	}

	ZString stgTemp = LOCKDIR;
	stgTemp += pName;
	while((*pfd = open(stgTemp, O_RDWR|O_CREAT|O_EXCL, 0644)) < 0)
		sleep(2);				// File is locked by other process, wait
}

void CleanDB (DBM * pFileDB, DBM * pMsgDB, datum * pKey, datum * pResp,
	char * pName, BOOL bRemove)
{
	// Loop through the messages IDs and delete them
	mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
	char * p = strrchr(pName, '/');
	p++;
	LockFile(LockFileName=p, &LockFD);
	if(pResp->dsize)
	{
		ZString szMsg(pResp->dptr, pResp->dsize);
		char * p = szMsg;
		char * q;
		datum key;
		LockFile(szMessageID, &LockMsgFD);		// lock the database
		while(p && *p)
		{
			q = strchr(p, '\t'); 
			if(q)
				*q++ = '\0';
			key.dptr = p;
			key.dsize = strlen(p);
			dbm_delete(pMsgDB, key);
			p = q;
		}
		dbm_close(pMsgDB);
		if((pMsgDB = dbm_open(szMsgName, O_RDWR, mode)) == NULL)
		{
			dbm_close(pFileDB);
			cout << "Cannot re-open database file " << szMsgName;
			cout << endl;
			exit(1);
		}
		UnLockFile(szMessageID, &LockMsgFD);
	}
	LockFile(szFileID, &LockFileFD);			// lock the database
	
	ZString szFile(pKey->dptr, pKey->dsize);
	dbm_delete(pFileDB, *pKey);
	dbm_close(pFileDB);
	if((pFileDB = dbm_open(szFileName, O_RDWR, mode)) == NULL)
	{
		dbm_close(pMsgDB);
		cout << "Cannot re-open database file " << szFileName;
		cout << endl;
		exit(1);
	}
	UnLockFile(szFileID, &LockFileFD);

	if(bRemove)
	{
		cout << "Removed file: " << pName << endl;
		unlink(pName);							// remove file
	}
	UnLockFile(LockFileName, &LockFD);
}

//
// Usage:: dbmclean [no of days}
//
main(int argc, char *argv[])
{
	if(argc > 2)
	{
		cout << "Usage: dbmclean [no of days]" << endl;
		exit(1);
	}

	int nDays = 10;
	if(argc == 2)
	{
		nDays = atoi(argv[1]);
		if(nDays == 0)
			nDays = 10;
	}
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	long nKeepTime = tv.tv_sec - nDays * 24 * 3600;
	szFileName = DECODEDIR;
	szFileName += "/";
	szFileName += &szFileID[0];
	mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
	DBM * pFileDB = dbm_open(szFileName, O_RDWR, mode);
  	if(pFileDB == NULL)
	{
		cout << "Cannot open database file " << szFileName << endl;
		exit(1);
	}
	szMsgName = DECODEDIR;
	szMsgName += "/";
	szMsgName += &szMessageID[0];
	DBM * pMsgDB = dbm_open(szMsgName, O_RDWR, mode);
  	if(pMsgDB == NULL)
	{
		dbm_close(pFileDB);
		cout << "Cannot open database file " << szMsgName << endl;
		exit(1);
	}

	struct stat sb;
	datum key, resp;
	int nSize = 512;
	char * pFile = new char[nSize];
	int nLen;
	for(key = dbm_firstkey(pFileDB); key.dptr != NULL; )
	{
		resp = dbm_fetch(pFileDB, key);
		if(key.dsize)
		{
			nLen = strlen(DECODEDIR);
			if((nLen+key.dsize) >= nSize)
			{
cout << "Buffer is not big enough" << endl;
exit(1);
			}
			strcpy(pFile, DECODEDIR);
			nLen = strlen(pFile);
			pFile[nLen++] = '/';
			strncpy(pFile+nLen, key.dptr, key.dsize);
			nLen += key.dsize;
			pFile[nLen] = '\0';
			if(stat(pFile, &sb) < 0)
			{
				// File is bad, delete it from database
				CleanDB(pFileDB, pMsgDB, &key, &resp, pFile, FALSE);
				key=dbm_nextkey(pFileDB);
				continue;
			}

			if(sb.st_ctime < nKeepTime)
			{
				// File hasn't been access for nDays, delete it from database
				CleanDB(pFileDB, pMsgDB, &key, &resp, pFile, TRUE);
				key=dbm_nextkey(pFileDB);
				continue;
			}
		}
		key=dbm_nextkey(pFileDB);
	}
	dbm_close(pMsgDB);
	dbm_close(pFileDB);
	delete [] pFile;
	exit(0);
}
