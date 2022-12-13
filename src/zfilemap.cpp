#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "def.h"

#include "zfilemap.h"

/*
	Function: ZFileMap::ZFileMap
	Description:
		Constructor for ZFileMap class.  The function allocates a
		buffer cSize in length.
*/
ZFileMap::ZFileMap(const unsigned nSize) :
	m_pData(NULL), m_nBufferSize(nSize),
	m_fdes(-1),m_iError(CMM_NO_ERROR), m_bIsOpen(FALSE)
{
}

/*
	Function: ZFileMap::~ZFileMap
	Description:
		Simply make sure the file is closed before we're destroyed.
*/
ZFileMap::~ZFileMap()
{
	// Make sure file is closed.
	Close();
}

/*
	Function: ZFileMap::Open
	Description:
		Open the file, allocate the buffer, and read the first bytes
		of the file into it.
*/
int ZFileMap::Open(char * pFileName)
{
	if((m_fdes = open(pFileName, O_RDONLY)) == -1)
		return m_iError = CMM_OPEN_ERROR;

	// Allocate the buffer.
    m_pData = new unsigned char[m_nBufferSize];

	// Read as much of file into buffer as possible, starting from
	// beginning of file.
	FillBuffer(0L);

	m_bIsOpen = MIME_TRUE;

	return m_iError = CMM_NO_ERROR;
}

/*
	Function: ZFileMap::Close
	Description:
		Delete the data buffer and close the file.
*/
int ZFileMap::Close(void)
{
	// Get rid of the buffer.
	delete[] m_pData;
	m_pData = NULL;

	if(m_fdes != -1)
	{
		if(close(m_fdes) == -1)
			return m_iError = CMM_NO_ERROR;
		m_fdes = -1;
	}

    m_bIsOpen = FALSE;

	return m_iError=CMM_NO_ERROR;
}

/*
	Function: ZFileMap::FileLength
	Description:
		Return the length of the file.
*/
long ZFileMap::FileLength()
{
	long lSeekPos = lseek(m_fdes, 0, SEEK_END);
	if(lSeekPos == -1)
		return m_iError = CMM_NO_ERROR;
	m_iError = CMM_NO_ERROR;
	return lSeekPos;
/*
	struct stat filest;
	fstat(m_fdes, &filest);
	m_iError = CMM_NO_ERROR;
	return filest.st_size;
*/
}

/*
	Function: ZFileMap::FillBuffer
	Description:
		Fill the buffer with file data starting at lBaseIndex
		within the file.
*/
int ZFileMap::FillBuffer(const long lBaseIndex)
{
	// Seek to correct position in file.
	long lSeekPos = lseek(m_fdes, lBaseIndex, SEEK_SET);
	if (lSeekPos == -1 || lSeekPos != lBaseIndex)
		return m_iError = CMM_SEEK_ERROR;

	// Read data in.
	int uResult = read(m_fdes, m_pData, m_nBufferSize);
	if (uResult == -1)
		return m_iError = CMM_READ_ERROR;

	// Zero fill remainder of buffer.
	if(uResult != (int)m_nBufferSize)
	{
		for (unsigned i = uResult; i < m_nBufferSize; i++)
			m_pData[i] = 0;
	}

	// Update buffer variables.
	m_lBaseIndex	= lBaseIndex;
	m_uLen			= uResult;

	return m_iError = CMM_NO_ERROR;
}

/*
	Function: ZFileMap::ReadAndGet
	Description:
		All access to the file data is through the Get member
		function.  The Get function is inlined for speed.  If the
		data is in the buffer, then the inlined code should execute
		quickly.  If the Get function determines that the desired
		data is not in the buffer, then it calls ReadAndGet to read
		the data into the buffer and return the appropriate data.
		Since reading the file is a slow operation, this code is
		put into its own function to save space in the inlined
		Get function.
*/
int ZFileMap::ReadAndGet(const long lIndex)
{
	// We didn't have the right data, so get the right data
	long lBaseIndex = (lIndex / m_nBufferSize) * m_nBufferSize;
	int iResult = FillBuffer(lBaseIndex);
	if (iResult != CMM_NO_ERROR) {
		return m_iError = iResult;
	}

	// Try again...
	unsigned uOffset = (unsigned) (lIndex - m_lBaseIndex);
	if (uOffset < m_uLen)
		return m_pData[uOffset];
	else
		return EOF;
}

inline int ZFileMap::Get (const long lIndex)
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
