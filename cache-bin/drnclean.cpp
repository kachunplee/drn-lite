#include <g++/iostream.h>
#include <g++/fstream.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <builtin.h>
#include <ndbm.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <String.h>
#include "drncache.h"

#define DECODEDIR "/var/local/etc/httpd/decoded/"
#define LOCKDECODEDIR "/var/local/etc/httpd/decoded/lock/"
#define DRNCLEAN "drnclean"
#define DRNCACHEFILE "/var/local/etc/httpd/drn-cache/drncache"
#define DRNCACHEDIR "/var/local/etc/httpd/drn-cache/"
#define MSGDBFILE "messageid"
#define DRNLOG "/var/tmp/drnclean.log"
#define DECODEFS "/n0"
/*
#define DECODEFS "/var"
*/
#define BUF_SIZE 10000
#define MSG_LIMIT 100
#define DEL_LIMIT 25

int fd_log = -1;

void write_log (char * p)
{
	if(p && *p)
	{
		if(fd_log == -1)
		{
			fd_log = open(DRNLOG, O_WRONLY|O_APPEND|O_CREAT, 0644);
			if(fd_log < 0)
				return; 
		}
		write(fd_log, p, strlen(p));
	}
}

void LockFile (char *pName, int nExit)
{
	String tmpStg = LOCKDECODEDIR;
	tmpStg += pName;
	int fd;
	pid_t filepid;
	pid_t pid = getpid();
	while((fd = open(tmpStg, O_RDWR|O_CREAT|O_EXCL, 0644)) < 0)
	{
		// file locked by another proces, check if process is still alived
		if((fd = open(tmpStg, O_RDONLY, 0644)) >= 0)
		{
			read(fd, (char *)(&filepid), sizeof(pid_t));
			close(fd);
			if(filepid == pid)
				return;		// locked by current process, done
			if(kill(filepid, 0) < 0)
			{
				// process that locked the file is no longer
				//	exist, unlink the file
				unlink(tmpStg);
				continue;
			}
		}

		// File is locked by other process, wait or exit
		if(nExit)
		{
			write_log("drnclean is running already\n");
			if(fd_log > -1)
				close(fd_log);
			exit(0);
		}
		sleep(2);
	}
	write(fd, (char *)(&pid), sizeof(pid_t));
	close(fd);
}

void UnLockFile (char *pName)
{
	String tmpStg = LOCKDECODEDIR;
	tmpStg += pName;
	pid_t pid;
	int fd;
	if((fd = open(tmpStg, O_RDONLY, 0644)) > 0)
	{
		read(fd, (char *)(&pid), sizeof(pid_t));
		close(fd);
		if(pid == getpid())
			unlink(tmpStg);
	}
}

void CleanUp (int)
{
	UnLockFile(MSGDBFILE);
	UnLockFile(DRNCLEAN);
	if(fd_log > -1)
	{
		close(fd_log);
	}
}

main()
{
	char msgBuf[2048];

	struct timeval tp;
	struct timezone tzp;
	gettimeofday(&tp, &tzp);
	write_log(ctime((const time_t *)&(tp.tv_sec)));

	LockFile(DRNCLEAN, 1);

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

	int nBufLen = BUF_SIZE;
	char * pBuffer = new char[nBufLen];
	char * pBuf;
	char * p, * q;
	char * pMsgs[MSG_LIMIT];
	int nWrap = 0;
	int nCount;
	int sLen, rLen, nLen, i, j;
	DBM * pMsgDB;
	datum key;
	String tmpStg;
	int fd;
	for(;;)
	{
		while((fd = open(DRNCACHEFILE, O_RDWR|O_EXLOCK, 0644)) < 0)
		{
			if(errno == ENOENT)
			{
				write_log("DRN cache file is not exist\n");
			}
			sleep(2);
		}

		flock(fd, LOCK_EX);
		struct CacheFileInfo info;
		read(fd, (char *)&info, sizeof(CacheFileInfo));
		if(info.MagicNo != CACHE_MAGICNO)
		{
			flock(fd, LOCK_UN);
			close(fd);
			write_log("Bad MagicNo in cache file\n");
			CleanUp(0);
			exit(0);
		}

		pBuf = pBuffer;
		nWrap = nLen = 0;
		lseek(fd, info.BegOffset, SEEK_SET);
		for(nCount = 0; nCount < MSG_LIMIT; nCount++)
		{
			if(info.BegOffset == info.EndOffset)
				break;
			if((CACHE_SIZE - info.BegOffset) < (int)sizeof(int))
			{
				info.BegOffset = sizeof(CacheFileInfo);
				if(nWrap > 0)
				{
					write_log("Wrap through cache file more than once\n");
					break;
				}
				lseek(fd, info.BegOffset, SEEK_SET);
				nWrap++;
			}
			read(fd, (char *)&sLen, sizeof(int));
			if(sLen > (nBufLen - nLen))
			{
				// not enough to hold the next entry
				if(nCount > 0)
					break;	// clean the buffered entries

				// no entry in buffer yet, increase buffer size
				delete [] pBuffer;
				nBufLen = sLen + BUF_SIZE;
				pBuffer = new char[nBufLen];
				if(pBuffer == NULL)
				{
					sprintf(msgBuf, "Can't allocate buffer size %d\n", nBufLen);
					write_log(msgBuf);
					break;
				}
				pBuf = pBuffer;
			}
			rLen = 0;
			info.BegOffset += sizeof(int);
			if((CACHE_SIZE - info.BegOffset) < sLen)
			{
				rLen = CACHE_SIZE - info.BegOffset;
				read(fd, pBuf, rLen);
				info.BegOffset = sizeof(CacheFileInfo);
				lseek(fd, info.BegOffset, SEEK_SET);
				nWrap++;
			}
			read(fd, pBuf+rLen, sLen-rLen);
			pMsgs[nCount] = pBuf;
			pBuf[sLen] = 0;
			pBuf += sLen + 1;
			nLen += sLen + 1;
			info.BegOffset += (sLen - rLen);
		}
		lseek(fd, 0, SEEK_SET);
		write(fd, (char *)&info, sizeof(CacheFileInfo));
		flock(fd, LOCK_UN);
		close(fd);

		if(nCount == 0)
			break;

		i = 0;
		while(nCount)
		{
			LockFile(MSGDBFILE, 0);
			tmpStg = DRNCACHEDIR;
			tmpStg += MSGDBFILE;
			pMsgDB = dbm_open(tmpStg, O_RDWR,
				S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
			if(pMsgDB == NULL)
			{
				sprintf(msgBuf, "Can't open databse %s\n",
					tmpStg.chars());
				write_log(msgBuf);
				CleanUp(0);
				exit(0);
			}
			for(j = 0; j < DEL_LIMIT && nCount; j++, i++)
			{
				if(*(pMsgs[i]) == 0)
					continue;
				write_log(pMsgs[i]);
				write_log("\n");
				p = strchr(pMsgs[i], ' ');
				if(p)
					*p++ = 0;
				while(p && *p)
				{
					q = strchr(p, ' ');
					if(q)
						*q++ = 0;
					key.dptr = p;
					key.dsize = strlen(p);
					dbm_delete(pMsgDB, key);
					p = q;
				}
				tmpStg = DECODEDIR;
				tmpStg += pMsgs[i];
				unlink(tmpStg);
				nCount--;
			}
			dbm_close(pMsgDB);
			UnLockFile(MSGDBFILE);
			if(nCount)
				sleep(2);	// Give other process a chance
		}

		// Check if need to clean more files
		i = info.BegOffset - info.EndOffset;
		if(i < 1)
			i += CACHE_SIZE;
		if(i > CACHE_MIN)
		{
			// Check for disk space - later
			struct statfs sfs;
			if(statfs(DECODEFS, &sfs) < 0)
			{
				write_log("file system stat error\n"); 
				break;
			}

/*
			sprintf(msgBuf, "file system %s has %d%c free space\n",
				DECODEFS, (int)((sfs.f_bavail*100)/sfs.f_blocks), 0x25);
			write_log(msgBuf);
				break;
*/
			if(((sfs.f_bavail * 100) / sfs.f_blocks) > 20)
				break;
		}
		sleep(2);		// Give other process a chance
	}
	UnLockFile(DRNCLEAN);
}
