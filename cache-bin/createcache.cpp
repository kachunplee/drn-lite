#include <g++/iostream.h>
#include <g++/fstream.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "drncache.h"

#define DRNCACHEFILE "drncache"
#define TMP_SIZE 100000

main()
{
	char pBuf[TMP_SIZE];
	memset(pBuf, ' ', TMP_SIZE);

	int fd;
	if((fd = open(DRNCACHEFILE, O_RDWR|O_CREAT|O_TRUNC|O_EXLOCK, 0644)) < 0)
	{
		if(errno == ENOENT)
		{
			cout << "Can't create DRN cache file\n";
			exit(0);
		}
		cout << "Create DRN cache file error" << errno << "\n";
		exit(0);
	}

	struct CacheFileInfo info;
	info.MagicNo = CACHE_MAGICNO;
	info.BegOffset = info.EndOffset = sizeof(CacheFileInfo);
	write(fd, (char *)&info, sizeof(CacheFileInfo));

	int rLen;
	int nLen = CACHE_SIZE - sizeof(CacheFileInfo);
	for( ; nLen > 0; nLen -= rLen)
	{
		rLen = TMP_SIZE;
		if(nLen < rLen)
			rLen = nLen;
		write(fd, pBuf, rLen);
	}
	close(fd);
	cout << "Create DRN cache successfully\n";
}
