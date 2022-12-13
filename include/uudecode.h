#ifndef __UUDECODE_H__
#define __UUDECODE_H__

class UUDecode
{
protected:
	ofstream	m_stm;
	char *		m_pBuffer;
	int			m_nBufLen;

public:
	UUDecode ()		{ m_pBuffer = new char[m_nBufLen=256];}
	~UUDecode ()		{ delete [] m_pBuffer; }

	BOOL Open(const char *);
	void Close();
	void SetMode();
	void Decode(const char *);
};

#endif //__UUDECODE_H__
