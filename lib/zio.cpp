#include "def.h"

char * Zifstream::GetLine (int * pLen, int term)
{
	get(m_buf, term);
	char * p = m_buf.str();
	m_buf.Terminate(pLen);
	return p?p:(char *)"";
}

bool Zifstream::NextLine ()
{
	if(good())
	{
		get();										// Throw away Terminator
		m_buf.ResetPut();
		return true;
	}

	return false;
}
