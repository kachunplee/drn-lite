#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "dmsg.h"
#include "cnntp.h"
#include "nntpact.h"

extern const char * NNTPservers;
extern const char * NNTPauths;

NNTPActive::NNTPActive ()
{
	char * p;
	m_pNNTP = new CNNTP(NNTPservers, NNTPauths);

	p = m_pNNTP->SendCmd("LIST active\r\n");
	DMSG(1, "%s", p);
	if(strncmp(p, "215 ", 4) != 0) throw(p);
}

NNTPActive::~NNTPActive ()
{
	delete m_pNNTP;
}

char * NNTPActive::GetLine ()
{
	if(m_pNNTP == NULL)
		return NULL;

	return m_pNNTP->GetLine();
}
