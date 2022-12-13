#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "dmsg.h"
#include "cnntp.h"
#include "nntpart.h"

extern const char * NNTPservers;
extern const char * NNTPauths;

NNTPArticle::NNTPArticle (const char * group, const char * artnum, bool hdronly)
{
	char * p;
	m_pNNTP = new CNNTP(NNTPservers, NNTPauths);

	if(group[0] != '<' || group[strlen(group)-1] != '>')
	{
		m_pNNTP->SetCmd("group %s\r\n", group);
		p = m_pNNTP->SendCmd();
		DMSG(1, "%s", p);
		if(strncmp(p, "211 ", 4) != 0) throw(p);
	}
	else
		artnum = group;

	m_pNNTP->SetCmd("%s %s\r\n", hdronly?"HEAD":"ARTICLE", artnum);
	p = m_pNNTP->SendCmd();
	DMSG(1, "%s", p);
	if(strncmp(p, "220 ", 4) != 0) throw(p);
}

NNTPArticle::~NNTPArticle ()
{
	delete m_pNNTP;
}

char * NNTPArticle::GetLine ()
{
	if(m_pNNTP == NULL)
		return NULL;

	return m_pNNTP->GetLine();
}
