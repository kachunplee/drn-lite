#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>

int debug = 0;

void
DMSG (int priority, char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	if(priority <= debug)
	{
		vfprintf(stderr, fmt, args);
		putc('\n',stderr);
	}
}
