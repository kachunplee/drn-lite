#ifndef __ZIO_H__
#define __ZIO_H__

#include <iostream.h>
#include <fstream.h>

#include "zstg.h"

class Zifstream : public ifstream
{
protected:
	StringStreamBuf m_buf;

public:
    Zifstream ()											{}
    Zifstream (int fd) : ifstream(fd)						{}
    Zifstream (int fd, char *p, int l) : ifstream(fd, p, l)	{}
    Zifstream (const char *name, int mode=ios::in, int prot=0444)
		: ifstream(name, mode, prot) {}

	char * GetLine(int * pLen = NULL, int term = '\n');
	bool NextLine();
};

#endif // __ZIO_H__
