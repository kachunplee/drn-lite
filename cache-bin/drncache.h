#ifndef __DRNCACHE_H__
#define __DRNCACHE_H__

#define CACHE_MAGICNO	19970507
#define CACHE_SIZE	20000000
#define CACHE_MIN	2000000

struct CacheFileInfo
{
	int	MagicNo;
	off_t	BegOffset;
	off_t	EndOffset;
};

#endif //__DRNCACHE_H__
