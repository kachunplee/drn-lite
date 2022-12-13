#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "def.h"

char szMessageID[] =		"messageid";
ZString LockFileName;
BOOL bSignal = FALSE;

void InitSignals ()
{
	if(!bSignal)
	{
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
		bSignal = TRUE;
	}
}

void CleanUp (int)
{
	int fd;
	pid_t pid = getpid();
	pid_t filepid;
	ZString stgTemp;
	stgTemp = LOCKDECODEDIR;
	stgTemp += LockFileName;
	if((fd = open(stgTemp, O_RDONLY, 0644)) >= 0)
	{
		read(fd, (char *)(&filepid), sizeof(pid_t));
		close(fd);
		if(pid == filepid)
			unlink(stgTemp);
	}
	stgTemp = LOCKDECODEDIR;
	stgTemp += szMessageID;
	if((fd = open(stgTemp, O_RDONLY, 0644)) >= 0)
	{
		read(fd, (char *)(&filepid), sizeof(pid_t));
		close(fd);
		if(pid == filepid)
			unlink(stgTemp);
	}

	exit(0);
}
