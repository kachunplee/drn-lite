#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "def.h"
#include "base64.h"


// Calculate decoded character value.  Note that only legal
// characters should be given as input.
int Base64Decode::DecodeValue (const int c)
{
	// Note that this works only on ASCII machines.
	if ('A' <= c && c <= 'Z')
		return c - 'A';
	if ('a' <= c && c <= 'z')
		return c - 'a' + 26;
	if ('0' <= c && c <= '9')
		return c - '0' + 52;
	if (c == '+')
		return 62;
	if (c == '/')
		return 63;
	if (c == '=')
		return -1;
	return -2;
}


BOOL Base64Decode::Open (const char* pszFileName)
{
	m_iChars = 0;
	ZString FileName = DECODEDIR;
	FileName += pszFileName;

	return (m_pFile = fopen(FileName, "w")) != NULL;
}

void Base64Decode::Close ()
{
	if(m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

void Base64Decode::SetMode ()
{
	fchmod(fileno(m_pFile), 0644);
}

int Base64Decode::Decode (const char* pszLine)
{
	int nLineLen = strlen(pszLine);
	uchar * uchBuffer = new uchar[nLineLen];
	int nDecodedLen = DecodeLine(pszLine, nLineLen,
		uchBuffer, nLineLen);
	if (nDecodedLen >= 0)
	{
		int n = fwrite(uchBuffer, sizeof(uchar), nDecodedLen, m_pFile);
		ASSERT(n==nDecodedLen);
		//m_stm << uchBuffer;
	}
	delete[] uchBuffer;
    return nDecodedLen;
}

int Base64Decode::DecodeLine (const char* pszLine, int nLineLen,
	uchar* uchBuffer, int nBufferLen)
{
	int iLineIndex = 0;
	int iBufferIndex = 0;

	while (iLineIndex < nLineLen) {
		// Group together four characters for decode.
		while (iLineIndex < nLineLen && m_iChars < 4) {
			int c = pszLine[iLineIndex++];
			// Ignore characters that aren't BASE64 characters
            // (e.g., spaces, CRLF, etc.).
			if (DecodeValue(c) != -2)
				m_uchStoredChars[m_iChars++] = (uchar) c;
		}

		if (m_iChars == 4) {
			// We've got four characters, so decode them.
			m_iChars = 0;

			// Decode first byte.
			if (iBufferIndex == nBufferLen) return -1;
			uchBuffer[iBufferIndex++] = (uchar)
				(((uchar)DecodeValue(m_uchStoredChars[0]) << 2) |
				((uchar)DecodeValue(m_uchStoredChars[1]) >> 4));

			// Decode second byte.
			if (iBufferIndex == nBufferLen) return -1;
			if (m_uchStoredChars[2] == '=') return iBufferIndex;
			uchBuffer[iBufferIndex++] = (uchar)
				(((uchar)DecodeValue(m_uchStoredChars[1]) << 4) |
				((uchar)DecodeValue(m_uchStoredChars[2]) >> 2));

			// Decode third byte.
			if (iBufferIndex == nBufferLen) return -1;
			if (m_uchStoredChars[3] == '=') return iBufferIndex;
			uchBuffer[iBufferIndex++] = (uchar)
				(((uchar)DecodeValue(m_uchStoredChars[2]) << 6) |
				((uchar)DecodeValue(m_uchStoredChars[3])));
		}
	}

    // Return the count of decoded bytes.
	//uchBuffer[iBufferIndex++] = 0;
    return iBufferIndex;
}
