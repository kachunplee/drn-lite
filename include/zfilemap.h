#ifndef __ZFILEMAP_H
#define __ZFILEMAP_H

#include <stdio.h>	// Just need EOF.
#include <sys/time.h>

#define MIME_TRUE				1

/** CONSTANTS **/
#define DEFAULT_BUFFER_SIZE		1024
// Errors
#define	CMM_NO_ERROR			0
// Note that EOF is not really an error but is returned as one
// by Get.  It should be equal to -1, but you never know.  If it's
// not -1, then it might clash with some of the other errors.
#if (EOF != -1)
#error The end of file constant value is not -1.
#endif
#define CMM_NO_MEMORY			-2
#define CMM_OPEN_ERROR			-3
#define CMM_WRITE_ERROR			-4
#define CMM_READ_ERROR			-5
#define CMM_CLOSE_ERROR			-6
#define CMM_SEEK_ERROR			-7

#define CMM_ERROR_MIN			-7		// min error


/*
	Class: ZFileMap
	Description:
		Implements a file mapping object.
*/
class ZFileMap {
private:
	// Variables
	unsigned char*	m_pData;		// Pointer to file data
	unsigned	m_nBufferSize;	// Total size of buffer
	int			m_fdes;			// File description
	long		m_lBaseIndex;	// File index of first byte in buffer
	unsigned	m_uLen;			// Length of data within the buffer
	int			m_iError;		// Most recent error code
	BOOL		m_bIsOpen;		// The file is open.

	// Private functions
	int FillBuffer(const long lBaseIndex);

public:
	// Constructor uses default buffer size if not provided.
	ZFileMap(const unsigned nSize = DEFAULT_BUFFER_SIZE);
	~ZFileMap();

	int Open(char *);
	int Close(void);
    BOOL IsOpen(void) { return m_bIsOpen; };
	long FileLength(void);
	int LastError(void) { return m_iError; };

	// Access Functions
	int Get(const long lIndex);
	int operator[](const long lIndex)
	{
		ASSERT(m_bIsOpen);

		// See if our buffer has the right data in it.
		if (m_lBaseIndex <= lIndex &&
			lIndex < (m_lBaseIndex + (long) m_nBufferSize)) {
			// Compute the offset of the byte in the buffer.
			unsigned uOffset = (unsigned) (lIndex - m_lBaseIndex);
			// Make sure we have that many bytes in the buffer.
			if (uOffset < m_uLen)
				return m_pData[uOffset];
			else
				return EOF;
		}
		else
    		return ReadAndGet(lIndex);
	}
	int ReadAndGet(const long lIndex);
};

#endif //__ZFILEMAP_H__
