#ifndef _BASE64_H
#define _BASE64_H

#include <stdio.h>

typedef unsigned char uchar;


class Base64Decode
{
protected:
	FILE*		m_pFile;
	uchar		m_uchStoredChars[4];
	int			m_iChars;

public:
	Base64Decode() { m_pFile = NULL; m_iChars = 0; }
	virtual ~Base64Decode() { Close(); };

	BOOL Open(const char* pszFileName);
	void Close();
	void SetMode();
    int Decode(const char* pszLine);
	int DecodeLine(const char* pszLine, int nLineLen,
		uchar* uchBuffer, int nBufferLen);

	int DecodeValue(const int c);
};


#endif //_BASE64_H
