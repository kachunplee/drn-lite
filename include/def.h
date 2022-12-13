#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

typedef unsigned char BYTE;

typedef signed char SBOOL;

#if 1
typedef bool BOOL;
#define TRUE true
#define FALSE false
#else
typedef int BOOL;
const int FALSE = 0;
const int TRUE = -1;
#endif

typedef long   ARTNUM;

#define NGH_HASH(p, j) \
	for(j = 0; *p; ) j = (j << 5) + j + *p++

#include "conf.h"
#include "dmsg.h"

#include "zio.h"
#include "htmllib.h"
#include "urllib.h"
#include "inlib.h"

#include "dmsg.h"
#include "newsopt.h"

#ifdef DEBUG
#define ASSERT assert
#else
void ASSERT (int);
void ASSERT (char *);
#endif

extern char szMessageID[];
extern ZString LockFileName;
extern BOOL bSemLock;
extern int SemId;
extern struct sembuf op_lock[];
extern struct sembuf op_unlock[];
extern void InitSignals();
extern void CleanUp(int);

extern "C"
{
	int wildmat(const char * text, const char * p);
};
