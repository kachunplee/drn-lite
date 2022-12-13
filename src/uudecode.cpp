#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "def.h"

#include "uudecode.h"

BOOL UUDecode::Open (const char * pFileName)
{
	ZString FileName = DECODEDIR;
	FileName += pFileName;
	m_stm.open(FileName, ios::out);
	return m_stm.is_open();
}

void UUDecode::Close ()
{
	m_stm.close();
}

void UUDecode::SetMode ()
{
	fchmod(m_stm.rdbuf()->fd(), 0644);
}

#define	DEC(c)	(((c) - ' ') & 077)		/* single character decode */

void UUDecode::Decode (const char * p)
{
	int n;
	if ((n = DEC(*p)) <= 0)
		return;

	if(n > m_nBufLen)
	{
		delete [] m_pBuffer;
		m_pBuffer = new char[m_nBufLen=n];
	}
	int nLen = 0;
	for (++p; n > 0; p += 4, n -= 3)
	{
		if (n >= 3)
		{
			m_pBuffer[nLen++] = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
			m_pBuffer[nLen++] = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
			m_pBuffer[nLen++] = DEC(p[2]) << 6 | DEC(p[3]);
		}
		else
		{
			if (n >= 1)
				m_pBuffer[nLen++] = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
			if (n >= 2)
				m_pBuffer[nLen++] = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
			if (n >= 3)
				m_pBuffer[nLen++] = DEC(p[2]) << 6 | DEC(p[3]);
		}
	}
	m_stm.write(m_pBuffer, nLen);
}
