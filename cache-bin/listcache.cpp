#include <g++/iostream.h>
#include <g++/fstream.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "drncache.h"

#define DRNCACHEFILE "/var/local/etc/httpd/drn-cache/drncache"
#define TMP_SIZE 2048

main()
{
	int fd;
	while((fd = open(DRNCACHEFILE, O_RDONLY, 0644)) < 0)
	{
		if(errno == ENOENT)
		{
			cout << "DRN cache file is not exist\n";
		}
		sleep(2);
	}

	struct CacheFileInfo info;
	read(fd, (char *)&info, sizeof(CacheFileInfo));
	if(info.MagicNo != CACHE_MAGICNO)
	{
		cout << "Bad MagicNo in cache file\n";
		exit(0);
	}

	int nBeg = info.BegOffset;
	int nEnd = info.EndOffset;
	int nBufLen = TMP_SIZE;
	char * pBuf = new char[nBufLen];
	int nWrap = 0;
	int sLen, rLen, n;
	while(nBeg != nEnd)
	{
		if((CACHE_SIZE - nBeg) < (int)sizeof(int))
		{
			if(nWrap > 0)
			{
				cout << "Wrap through cache file more than once\n";
				exit(0);
			}
			nBeg = sizeof(CacheFileInfo);
			nWrap++;
		}
		lseek(fd, nBeg, SEEK_SET);
		nBeg += sizeof(int);
		read(fd, (char *)&sLen, sizeof(int));
		if(sLen >= nBufLen)
		{
			delete [] pBuf;
			nBufLen = sLen + 1;
			pBuf = new char[nBufLen];
			if(pBuf == NULL)
			{
				cout << "Can't allocate buffer size "
					<< nBufLen << "\n";
				exit(0);
			}
		}
		rLen = 0;
		if((CACHE_SIZE - nBeg) < sLen)
		{
			rLen = CACHE_SIZE - nBeg;
			n = read(fd, pBuf, rLen);
			if(n > 0)
			{
				pBuf[n] = 0;
				cout << pBuf;
			}
			nBeg = sizeof(CacheFileInfo);
			lseek(fd, nBeg, SEEK_SET);
			nWrap++;
		}
		n = read(fd, pBuf, sLen-rLen);
		if(n > 0)
			pBuf[n] = 0;
		cout << pBuf << "\n";
		nBeg += (sLen - rLen);
	}
}
