#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "dmsg.h"
#include "cnntp.h"
#include "nntpov.h"

extern const char * NNTPservers;
extern const char * NNTPauths;

NNTPOverview::NNTPOverview (const char * group)
{
	char * p;
	m_pNNTP = new CNNTP(NNTPservers, NNTPauths);

	m_pNNTP->SetCmd("GROUP %s\r\n", group);
	p = m_pNNTP->SendCmd();
	DMSG(1, "%s", p);
	if(strncmp(p, "211 ", 4) != 0) throw(p);

	if(sscanf(p+4, "%d %d %d", &m_numArt, &m_minArt, &m_maxArt) != 3)
		throw(p);

	if(m_numArt == 0)
	{
		delete m_pNNTP;
		m_pNNTP = NULL;
		return;
	}

	m_pNNTP->SetCmd("XOVER %d-%d\r\n", m_minArt, m_maxArt);
	m_pNNTP->SendCmd();
	DMSG(1, "%s", p);
	if(strncmp(p, "224 ", 4) != 0) throw(p);
}

NNTPOverview::~NNTPOverview ()
{
	delete m_pNNTP;
}

char * NNTPOverview::GetLine ()
{
	if(m_pNNTP == NULL)
		return NULL;

	return m_pNNTP->GetLine();
}
