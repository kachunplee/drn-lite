#include <string.h>

#include "def.h"

#include "rfc822.h"
#include "newsmime.h"
#include "newsart.h"

#define UNUSED_PARAMETER(x) x

/*********************************************************************
	Class: MIMEParamTuple
*********************************************************************/

/*
	Function: MIMEParamTuple::MIMEParamTuple
	Description:
		Copy constructor.  Make sure that we do more than a simple
		member-wise copy.  We need to actually duplicate the
		strings in the tuple.
*/
MIMEParamTuple::MIMEParamTuple(const MIMEParamTuple& tuple) :
	m_pszParam(NULL), m_pszValue(NULL)
{
	Param(tuple.m_pszParam);
	Value(tuple.m_pszValue);
}

/*
	Function: MIMEParamTuple::~MIMEParamTuple
	Description:
		Delete the param and value strings before the MIMEParamTuple
		object is destroyed itself.
*/
MIMEParamTuple::~MIMEParamTuple()
{
	delete[] m_pszParam;
	delete[] m_pszValue;
}

/*
	Function: MIMEParamTuple::Param
	Description:
		Makes a local copy of the parameter string and stores it
		in the MIMEParamTuple.
*/
void MIMEParamTuple::Param(const char * pszParam)
{
	delete[] m_pszParam;
	if (pszParam != NULL) {
		m_pszParam = new char[strlen(pszParam) + 1];
		strcpy(m_pszParam, pszParam);
	}
	else
		m_pszParam = NULL;
}

/*
	Function: MIMEParamTuple::Value
	Description:
		Makes a local copy of the value string and stores it
		in the MIMEParamTuple.
*/
void MIMEParamTuple::Value(const char * pszValue)
{
	delete[] m_pszValue;
	if (pszValue != NULL) {
		m_pszValue = new char[strlen(pszValue) + 1];
		strcpy(m_pszValue, pszValue);
	}
	else
    	m_pszValue = NULL;
}

/*
	Function: MIMEParamTuple::operator=
	Description:
		The overloaded assignment operator for the MIMEParamTuple
		class.
*/
MIMEParamTuple& MIMEParamTuple::operator=
	(const MIMEParamTuple& tuple)
{
	if (this != &tuple) {
		delete[] m_pszParam;
		delete[] m_pszValue;
		Param(tuple.m_pszParam);
        Value(tuple.m_pszValue);
	}
	return *this;
}


/*********************************************************************
	Class: MIMEParamList
*********************************************************************/

/*
	Function: MIMEParamList::~MIMEParamList
	Description:
		Deletes the pararm tuples before the list is destroyed.
*/
MIMEParamList::~MIMEParamList()
{
	MIMEParamTuple * pTuple = First();
	while (pTuple != NULL) {
		delete pTuple;
		pTuple = Next();
	}
}

/*
	Function: MIMEParamList::Compare
	Description:
		Simply returns 0.  A MIMEParamList is unordered.  Returning
		0 causes an Insert operation to stop at the first element,
		which results in the element being stored at the beginning
		of the list.
*/
int MIMEParamList::Compare(void *, void *)
{
	//UNUSED_PARAMETER(pvThing1);
	//UNUSED_PARAMETER(pvThing2);

	return 0;
}

/*
	Function: MIMEParamList::DeleteData
	Description:
		Deletes an element of the list, in this case, a
		MIMEParamTuple object.
*/
void MIMEParamList::DeleteData(void * pvThing)
{
	delete ((MIMEParamTuple *) pvThing);
}

/*
	Function: MIMEParamList::Add
	Description:
		Add a tuple to the end of the list.  A copy of the tuple
		is made before it is added.
*/
void MIMEParamList::Add(MIMEParamTuple * pTuple)
{
	MIMEParamTuple * pNewTuple = new MIMEParamTuple(*pTuple);
	ZList::Add(pNewTuple);
}

/*
	Function: MIMEParamList::Insert
	Description:
		Insert a tuple into the list.  Because a MIMEParamList is
		unordered, the tuple is inserted at the front of the list.
		A copy of the tuple is made before it is added.
*/
void MIMEParamList::Insert(MIMEParamTuple * pTuple)
{
	MIMEParamTuple * pNewTuple = new MIMEParamTuple(*pTuple);
	ZList::Insert(pNewTuple);
}

MIMEParamTuple* MIMEParamList::GetParam (const char* pszString)
{
	MIMEParamTuple* pTuple;
	for( pTuple = First(); pTuple; pTuple = Next() )
		if (!strcasecmp(pTuple->Param(), pszString))
			break;
	return pTuple;
}


/*********************************************************************
	Class: MIMEBodyPart
*********************************************************************/

/*
	Function: MIMEBodyPart::MIMEBodyPart
	Description:
		Constructor for MIMEBodyPart.  Just a huge initialization
		list for this one.
*/
MIMEBodyPart::MIMEBodyPart(MIMEBodyPart * pParent, const char * pszFilename,
	long lStart, long lEnd) :
	m_bEmpty(TRUE), m_bDigest(FALSE), m_pszFilename(DuplicateString(pszFilename)), 
	m_pszType(NULL), m_pszSubType(NULL), 
	m_pParamList(new MIMEParamList), m_pszEncoding(NULL), 
	m_pszDisposition(NULL), m_lStart(lStart), m_lEnd(lEnd), 
	m_pDispFileName(NULL), m_pSubject(NULL), m_pMessageID(NULL),
    m_pParent(pParent), m_pFirstChild(NULL), 
	m_pPrevSibling(NULL), m_pSibling(NULL), m_nPart(-1)
{
}

/*
	Function: MIMEBodyPart::~MIMEBodyPart
	Description:
		Delete all the memory we allocated during this process.
*/
MIMEBodyPart::~MIMEBodyPart()
{
	Close();

	delete[]	m_pszFilename;
	delete[]	m_pszType;
	delete[]	m_pszSubType;
	delete		m_pParamList;
	delete[]	m_pszEncoding;
	delete[]	m_pszDisposition;
	delete[]	m_pDispFileName;
	delete[]	m_pSubject;
	delete[]	m_pMessageID;

	// Delete all the children.
	MIMEBodyPart *pChild, *pNextChild;
	pChild = m_pFirstChild;
	while (pChild != NULL) {
		pNextChild = pChild->m_pSibling;
		delete pChild;
		pChild = pNextChild;
	}
}

/*
	Function: MIMEBodyPart::Open
	Description:
		Open the body part for parsing and reading.
*/
BOOL MIMEBodyPart::Open(void)
{
	// Simply open the filemap object.
	return (m_filemap.Open(m_pszFilename) == CMM_NO_ERROR) ?
		MIME_TRUE : FALSE;
}

/*
	Function: MIMEBodyPart::Close
	Description:
		Close the body part.  No more parsing or reading should
		be performed after this point.
*/
void MIMEBodyPart::Close(void)
{
	// Simply close the filemap object.
    m_filemap.Close();
}

/*
	Function: MIMEBodyPart::Parse
	Description:
		Parse the open body part.  The function first parses the
		headers.  If the headers indicate that the body part is
		multipart, then find the delimiters between the subparts and
		parse the subparts recursively.
*/
BOOL MIMEBodyPart::Parse(ostream & stm, BOOL * bMultipart)
{
	if (!m_filemap.IsOpen()) {
		return FALSE;
	}

	// Correct the values of start and end if they're set to
	// the defaults.
	if (m_lStart < 0)
		m_lStart = 0;
	if (m_lEnd < 0) {
		// Set lEnd to number of bytes in the file.
		m_lEnd = m_filemap.FileLength();
	}
	if( (m_lStart == 0) && (m_lEnd == m_filemap.FileLength()) )
		m_nPart = 0;

	// Get all the headers of interest.
	ParseHeaders(stm);

	// If this is a multipart message, parse it recursively.
	*bMultipart = FALSE;
	if (strcasecmp(m_pszType, "multipart") == 0) {
		if (ParseMultipart(stm) != MIME_TRUE)
        	return FALSE;
		else
			*bMultipart = TRUE;
	}

	return MIME_TRUE;
}

BOOL MIMEBodyPart::ParseHead(ostream & stm, BOOL * bMultipart)
{
	if (!m_filemap.IsOpen()) {
		return FALSE;
	}

	// Correct the values of start and end if they're set to
	// the defaults.
	if (m_lStart < 0)
		m_lStart = 0;
	if (m_lEnd < 0) {
		// Set lEnd to number of bytes in the file.
		m_lEnd = m_filemap.FileLength();
	}
	if( (m_lStart == 0) && (m_lEnd == m_filemap.FileLength()) )
		m_nPart = 0;

	// Get all the headers of interest.
	ParseHeaders(stm);

	*bMultipart = FALSE;
	return MIME_TRUE;
}

/*
	Function: MIMEBodyPart::ParseMultipart
	Description:
		Use the specified boundary to locate the various body
		parts of the multipart message.  Create a MIMEBodyPart
		for each body part and recursively call MIMEBodyPart::Parse
		to parse the parts.  Returns MIME_TRUE if no errors occurred,
		or FALSE otherwise.
*/
BOOL MIMEBodyPart::ParseMultipart(ostream & stm)
{
	// Find the "boundary" parameter tuple.
	MIMEParamTuple * pTuple = m_pParamList->First();
	while (pTuple != NULL) {
		if (strcasecmp(pTuple->Param(), "boundary") == 0)
			break;
		pTuple = m_pParamList->Next();
	}
	if (pTuple == NULL)
		return FALSE;

	// Create the real boundary string.
	int nBoundaryLen = strlen(pTuple->Value());
	char * pszBoundary = new char[nBoundaryLen + 2 + 1];
	//char * pszBoundary = new char[nBoundaryLen + 3 + 1];
	strcpy(pszBoundary, "--");
	//strcpy(pszBoundary, "\n--");
	strcat(pszBoundary, pTuple->Value());

	// Search for boundaries.
	long lBoundaryStart = StringSearch(pszBoundary);
	BOOL bFoundStartBoundary = FALSE;
	long lBodyPartStart = 0;	// just so the compile won't complaint
	long lBodyPartEnd;
	while (lBoundaryStart != -1) {
		// Okay, we found a boundary.  Is it a terminating boundary.
		BOOL bEndingBoundary = FALSE;
		if (m_filemap[m_lCurrent] == '-' &&
			m_filemap[m_lCurrent + 1] == '-') {
			bEndingBoundary = MIME_TRUE;
		}
		// Find the end of the line.
		long lBoundaryEnd = FindNewLine(stm);
		if (lBoundaryEnd < 0) {
			delete[] pszBoundary;
			return FALSE;
		}
		lBoundaryEnd += 1;  // Jump over \n.
		if (lBoundaryEnd >= m_lEnd) {
			lBoundaryEnd = m_lEnd;
		}

		if (!bFoundStartBoundary) {
        	// We haven't seen a starting boundary before.
        	// See if we got an end before we got a start.
			if (bEndingBoundary) {
				delete[] pszBoundary;
				return FALSE;
			}
			lBodyPartStart = lBoundaryEnd;
			bFoundStartBoundary = MIME_TRUE;
		}
		else {
			// If we've already found a boundary before, then
			// this was a separating boundary or the terminating
			// boundary.
			lBodyPartEnd = lBoundaryStart;
			// Create a child body part and add it to the heirarchy.
			MIMEBodyPart * pBodyPart = new MIMEBodyPart(this,
				m_pszFilename, lBodyPartStart, lBodyPartEnd);
			if (m_pFirstChild == NULL) {
				m_pFirstChild = pBodyPart;
				pBodyPart->m_nPart = m_nPart + 1;
			}
			else {
				// Add it to the end of the sibling chain.
				MIMEBodyPart * pChild = LastChild();
				pChild->m_pSibling = pBodyPart;
				pBodyPart->m_pPrevSibling = pChild;
				pBodyPart->m_nPart = pChild->m_nPart + 1;
			}
			if( !strcasecmp(m_pszType, "multipart") && 
				!strcasecmp(m_pszSubType, "digest") )
				pBodyPart->SetDigest();

			// The next body part starts right after this boundary.
			lBodyPartStart = lBoundaryEnd;

			// If this was the terminating boundary, then we're
			// done.
			if (bEndingBoundary)
				break;
		}

		// Find the next boundary.
		lBoundaryStart = StringSearch(pszBoundary);
	}

	delete[] pszBoundary;
	if (lBoundaryStart == -1)
		return FALSE;

	// Now we've found all the parts.  Traverse the child list
	// and have each child body part parse itself.
	MIMEBodyPart * pChild = m_pFirstChild;
	BOOL bMultipart;
	while (pChild != NULL) {
		if (!(pChild->Open())) return FALSE;
		if (!(pChild->Parse(stm, &bMultipart))) return FALSE;
		pChild->Close();
        pChild = pChild->m_pSibling;
	}

	return MIME_TRUE;
}

/*
	Function: MIMEBodyPart::ParseHeaders
	Description:
		Parse the body part headers to determine the content type
		and encoding of the message.
*/
BOOL MIMEBodyPart::ParseHeaders(ostream & stm)
{
	char *pszHeader;
    BOOL bResult;
	BOOL bFoundContentType = FALSE;
    BOOL bFoundEncoding = FALSE;
    BOOL bFoundDisposition = FALSE;
	MIMEParamTuple* pTuple = NULL;
	long lCurrent = 0;

	// Start looking for headers at the start of the current
    // body part range.
	m_lCurrent = m_lStart;

	while ((pszHeader = GetNextHeader(stm)) != NULL) {
		if (strcmp(pszHeader, "") == 0) {
			delete[] pszHeader;
			break;
		}
		else if (HeaderCompare(pszHeader, "MIME-Version")) {
			bResult = ParseMIMEVersion(pszHeader);
			if (bResult != MIME_TRUE) {
				delete[] pszHeader;
				goto error;
			}
		}
		else if (HeaderCompare(pszHeader, "Content-Type")) {
			bResult = ParseContentType(pszHeader);
			if (bResult != MIME_TRUE) {
				delete[] pszHeader;
				goto error;
			}
			bFoundContentType = MIME_TRUE;
		}
		else if (HeaderCompare(pszHeader, "Content-Transfer-Encoding")) {
			bResult = ParseContentTransferEncoding(pszHeader);
			if (bResult != MIME_TRUE) {
				delete[] pszHeader;
				goto error;
			}
			bFoundEncoding = MIME_TRUE;
		}
		else if (HeaderCompare(pszHeader, "Content-Disposition")) {
			bResult = ParseContentDisposition(pszHeader);
			if (bResult != MIME_TRUE) {
				delete[] pszHeader;
				goto error;
			}
			bFoundDisposition = MIME_TRUE;
		}
		if (HeaderCompare(pszHeader, "Subject"))
			m_pSubject = pszHeader;
		else if (HeaderCompare(pszHeader, "Message-ID"))
			m_pMessageID = pszHeader;
		else
			delete[] pszHeader;
	}

	// Fill in the defaults if the appropriate headers were missing.
	if (!bFoundContentType) {
		if (m_bDigest) {
			m_pszType = DuplicateString("message");
			m_pszSubType = DuplicateString("rfc822");
		}
		else {
			m_pszType = DuplicateString("text");
			m_pszSubType = DuplicateString("plain");
		}
	}
	if (!bFoundEncoding) {
		m_pszEncoding = DuplicateString("7bit");
	}

	if ((pTuple = ParamList()->GetParam("filename")))
		m_pDispFileName = DuplicateString(pTuple->Value());
	else if ((pTuple = ParamList()->GetParam("name")))
		m_pDispFileName = DuplicateString(pTuple->Value());
	if (!m_pDispFileName)
	{
		if( !strcasecmp(m_pszType, "message") && 
			!strcasecmp(m_pszSubType, "partial") )
		{
			ParseSubject(stm);
		}
		else if (m_pPrevSibling)
		{
			if( m_pPrevSibling->DispFileName() && m_pPrevSibling->IsEmpty() )
				m_pDispFileName = DuplicateString(m_pPrevSibling->DispFileName());
		}
		else if (m_pParent)
		{
			// Get file name from the parent
			if( m_pParent->DispFileName() && m_pParent->IsEmpty() )
				m_pDispFileName = DuplicateString(m_pParent->DispFileName());
		}
		if (!m_pDispFileName)
		{
			ParseSubject(stm);
		}
		if (!m_pDispFileName)
		{
			//assign a filename
		}
	}
	if (m_pDispFileName)
		for( char* z=m_pDispFileName; *z; z++ )
			if( isspace(*z) || (*z == '#') )	*z = '_';
	lCurrent = m_lCurrent;
	while( (m_filemap[lCurrent] != EOF) && (lCurrent < m_lEnd) && 
		(m_filemap[lCurrent] != '-') ){
		if (!isspace(m_filemap[lCurrent++])) {
			m_bEmpty = FALSE;
			break;
		}
	}
	return MIME_TRUE;

error:
	// Fill in the defaults if the appropriate headers were missing.
	if (!bFoundContentType) {
		if (m_bDigest) {
			m_pszType = DuplicateString("message");
			m_pszSubType = DuplicateString("rfc822");
		}
		else {
			m_pszType = DuplicateString("text");
			m_pszSubType = DuplicateString("plain");
		}
	}
	if (!bFoundEncoding) {
		m_pszEncoding = DuplicateString("7bit");
	}
	return FALSE;
}

void MIMEBodyPart::ParseSubject(ostream &)
{
	char * p = NULL;
	char * q, * s;
	int nExt, nName;
	MIMEBodyPart* pParent = m_pParent;
	if (m_pSubject)
		p = m_pSubject;
	else
		while (pParent)
		{
			if (pParent->m_pSubject)
			{
				p = pParent->m_pSubject;
				break;
			}
			pParent = pParent->m_pParent;
		}
	if (!p)
		return;

	while((q = strchr(p, '.'))) 
	{
		nExt = 1;
		s = q+1;

		// Get length of extension part
		while(*s && isalnum(*s))
		{
			nExt++;
			s++;
		}

		if(nExt <= 1)
		{
			p = s;
			continue;
		}

		// Found extension, get the name
		nName = 0;
		s = q-1;
		while( (s >= p) && !isspace(*s) && 
			(*s != '{') && (*s != '}') && 
			(*s != '[') && (*s != ']') && 
			(*s != '(') && (*s != ')') )
		{
			nName++;
			s--;
		}
		s++;

		if(nName == 0)
		{
			p = q + 1;
			continue;
		}

		// Found name
		nName += nExt;
		p = new char[nName+1];
		int i;
		for(i = 0; i < nName; i++)
			p[i] = s[i];
		p[i] = '\0';
		m_pDispFileName = p;
		return;
	}
}

/*
	Function: MIMEBodyPart::GetNextHeader
	Description:
		Find the next header in the file starting at m_lCurrent.
		Returns a newly created string with the header in it.
		The caller is responsible for deleteing the string when
		it is finished with the header.  The returned header has
		been unfolded and any trailing whitespace has been removed.
		All internal \n have been converted to a single space.
		The final \n has been removed.
*/
char * MIMEBodyPart::GetNextHeader(ostream & stm)
{
	long lEndPos;
	long lStartPos = m_lCurrent;

	// Find the first \n that doesn't have linear whitespace
	// following it.
	do {
		lEndPos = FindNewLine(stm);
		// Move the current position up.
		if (lEndPos >= 0) {
			m_lCurrent = lEndPos + 1;
		}
		if( (m_lCurrent - lStartPos == 1L) && (m_filemap[lStartPos] == '\n') )
			return NULL;
	} while (lEndPos >= 0 && IsLWSP(m_filemap[lEndPos + 1]));

    // If we couldn't find \n or an error occurred.
	if (lEndPos < 0)
	{
		return NULL;
	}

	// Push lEndPos to indicate the character following the \n.
	lEndPos += 1;

	// Calculate the header length.
	int nLen = (int)(lEndPos - lStartPos);
	ASSERT(nLen > 0);

	// Allocate a character string for it.
	char * pszHeader = new char[nLen + 1];
	// Copy the header to the buffer.  Replace \n with a
	// single space character.
	int iCopyIndex = 0;
	for (int i = 0; i < nLen; i++) {
		char c = (char) m_filemap[lStartPos + i];
		// If we've just seen a CR, see if the next character is
		// an LF.
		if(c == '\n')
			pszHeader[iCopyIndex++] = ' ';
		else
			pszHeader[iCopyIndex++] = c;
	}

	pszHeader[iCopyIndex] = '\0';

	// Remove any trailing whitespace and terminate the string.
	if(iCopyIndex > 0)
	{
		iCopyIndex--;
		while (iCopyIndex > -1 && IsLWSP(pszHeader[iCopyIndex]))
			iCopyIndex--;
		iCopyIndex++;
	}
	pszHeader[iCopyIndex] = '\0';

	return pszHeader;
}

/*
	Function: MIMEBodyPart::FindNewLine
	Description:
		Finds the next \n in the file after the position
		specified by m_lCurrent.  Returns the index in the file
		of the \n character.  Returns -1 if no \n can be found.
*/
long MIMEBodyPart::FindNewLine(ostream &)
{
	long lSearch = m_lCurrent;

	while (1)
	{
		int c = m_filemap[lSearch];
		if (c < 0)  // Error or EOF
		{
			return -1;
		}
		else if(c == '\n')
		{
			return lSearch;  // Found it.
		}
		lSearch++;
	}
}

/*
	Function: MIMEBodyPart::StringSearch
	Description:
		Searches for a string in the message text starting at
		m_lCurrent and ending at m_lEnd.  Returns the position of
		the first character in the string and advances m_lCurrent
		to the character following the string.  The function returns
		-1 if the string cannot be found before m_lEnd is reached.

		The function uses a Boyer-Moore string search algorithm in
		order to reduce the number of calls to CFileMap::Get
		and move through the string quickly.  Note, this is a
		pretty confusing search algorithm if you aren't used to it.
		Look it up in a textbook if you want more background
		(Sedgewick, "Algorithms in C," or Abrash, "Zen of Code
		Optimization," for instance).
*/
long MIMEBodyPart::StringSearch(char * pszString)
{
	int	iAdvanceTable[256];
	int i;

	ASSERT(pszString != NULL);
	int nLen = strlen(pszString);

	// If we've been asked to match a zero-length string, match it
	// immediately.
	if (nLen == 0)
    	return m_lCurrent;

	// Fill in the the advance table with the default length.
	for (i = 0; i < 256; i++)
		iAdvanceTable[i] = nLen;
	// Fill in the corresponding positions in the advance table
	// with the positions of the characters in the search string.
	for (i = 0; i < nLen - 1; i++)
		iAdvanceTable[pszString[i]] = nLen - i - 1;

	int iAdvance = 1;
	long lSearchPos = m_lCurrent + nLen - 1;
	while (lSearchPos < m_lEnd) {
		// Search from end of string back to front
		for (i = 0; i < nLen; i++) {
			int c = m_filemap[lSearchPos - i];
			if (c < 0) return -1;
			if (tolower(c) != tolower(pszString[nLen - 1 - i])) {
				// We found a mismatch.  See how far we can
				// advance based on the mismatched character.
				iAdvance = iAdvanceTable[c] - i;
                // We always move forward, never back.
				if (iAdvance <= 0)
					iAdvance = 1;
				break;
			}
		}
		if (i == nLen) {
			// We found it.
            m_lCurrent = lSearchPos + 1;
			return lSearchPos - (nLen - 1);
		}
		else {
			// Didn't find it, so advance the tail a bit more.
			lSearchPos += iAdvance;
		}
	}

	// Didn't find it before we ran out of room.
	return -1;
}

/*
	Function: MIMEBodyPart::HeaderCompare
	Description:
		Compare a header with a header name.  If the header name
		equals the part of the header preceeding the first colon
		(and after whitespace has been stripped), then return TRUE,
		else return FALSE.  The comparison is case-insensitive.
*/
BOOL MIMEBodyPart::HeaderCompare(const char * pszHeader,
	char *pszHeaderName)
{
	// Find the first colon in the header.
	unsigned int i = 0;
	while (pszHeader[i] != ':')
		i++;
	// Now back up to skip any whitespace between the header
	// name and the colon.
	i--; // Skip back past colon.
	while (IsLWSP(pszHeader[i]))
		i--;
	i++; // i = length of header name.

	// Compare the lengths.
	if (i != strlen(pszHeaderName))
		return FALSE;

	// Compare the characters.
	for (unsigned int j = 0; j < i; j++)
		if (tolower(pszHeader[j]) != tolower(pszHeaderName[j]))
			return FALSE;

	return MIME_TRUE;
}

/*
	Function: MIMEBodyPart::ParseMIMEVersion
	Description:
		Parse the MIME version header.  The function looks for
		version 1.0.  If version 1.0 is found and the header is
		syntactically correct, the function returns MIME_TRUE else it
		returns FALSE.
*/
BOOL MIMEBodyPart::ParseMIMEVersion(char * pszHeader)
{
	ASSERT(pszHeader != NULL);

	// Allocate a token buffer.  The maximum token length could
	// be the size of the header minus the header name.  Allocate
	// one the size of the whole header, which is sure to be enough.
	int nTokenLen = strlen(pszHeader);
	char * pszToken = new char[nTokenLen];
	int iToken;

	// Move up to the colon separating the header name from
	// the header body.
	while (*pszHeader != ':' && *pszHeader != '\0')
		pszHeader++;
	if (*pszHeader == '\0') goto error;
	// Move past the colon.
	pszHeader++;

	// Look for the '1'.
	do {
		pszHeader = GetToken(FALSE, pszHeader, pszToken, nTokenLen,
			&iToken);
	} while (iToken == TOKEN_COMMENT);
	if (iToken != TOKEN_ATOM || strcmp(pszToken, "1") != 0)
		goto error;

	// Look for the '.'.
	do {
		pszHeader = GetToken(FALSE, pszHeader, pszToken, nTokenLen,
			&iToken);
	} while (iToken == TOKEN_COMMENT);
	if (iToken != TOKEN_SPECIAL || strcmp(pszToken, ".") != 0)
		goto error;

	// Look for the '0'.
	do {
		pszHeader = GetToken(FALSE, pszHeader, pszToken, nTokenLen,
			&iToken);
	} while (iToken == TOKEN_COMMENT);
	if (iToken != TOKEN_ATOM || strcmp(pszToken, "0") != 0)
		goto error;

	delete[] pszToken;
	return MIME_TRUE;

error:
	delete[] pszToken;
    return FALSE;
}

/*
	Function: MIMEBodyPart::ParseContentType
	Description:
		Parses a Content-Type header.  The header name must have
		been checked previously.  The type and subtype fields are
		parsed from the header and stored in the node.  If
		parameters are present, they are parsed and stored in the
		parameter list.  If everything goes alright, the function
		returns MIME_TRUE.  If an error occurs, it returns FALSE.
*/
BOOL MIMEBodyPart::ParseContentType(char * pszHeader)
{
	ASSERT(pszHeader != NULL);

	// Allocate a token buffer.  The maximum token length could
	// be the size of the header minus the header name.  Allocate
	// one the size of the whole header, which is sure to be enough.
	int nTokenLen = strlen(pszHeader);
	char * pszToken = new char[nTokenLen];
	int iToken;
	MIMEParamTuple tuple;

	// Move up to the colon separating the header name from
	// the header body.
	while (*pszHeader != ':' && *pszHeader != '\0')
		pszHeader++;
	if (*pszHeader == '\0') goto error;
	// Move past the colon.
	pszHeader++;

	// Get the type.
	do {
		pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
			&iToken);
	} while (iToken == TOKEN_COMMENT);
	if (iToken != TOKEN_MIMETOKEN)
		goto error;
	m_pszType = DuplicateString(pszToken);

	// Get the "/" separator between the type and subtype.
	do {
		pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
			&iToken);
	} while (iToken == TOKEN_COMMENT);
	if (iToken != TOKEN_TSPECIAL || *pszToken != '/')
		goto error;

	// Get the subtype.
	do {
		pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
			&iToken);
	} while (iToken == TOKEN_COMMENT);
	if (iToken != TOKEN_MIMETOKEN)
		goto error;
	m_pszSubType = DuplicateString(pszToken);

	// See if we have any parameters.
	do {
		pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
			&iToken);
	} while (iToken == TOKEN_COMMENT);
	while (iToken == TOKEN_TSPECIAL && *pszToken == ';') {
		// Yes, we have a parameter.
		// Parse a token for the parameter name.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
		if (iToken != TOKEN_MIMETOKEN)
			goto error;
		tuple.Param(pszToken);
		// Get the equals sign.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
		if (iToken != TOKEN_TSPECIAL || *pszToken != '=')
			goto error;
		// Get the parameter value.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
		if (iToken != TOKEN_MIMETOKEN && iToken != TOKEN_QSTRING)
			goto error;
		tuple.Value(pszToken);
		// Add the tuple to the list of parameters.
		m_pParamList->Add(&tuple);
		// See if have another following ';'.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
	}

	// Okay, we should be at the end.
	delete[] pszToken;
	if (iToken != TOKEN_END)
		return FALSE;
	else
		return MIME_TRUE;

error:
	delete[] pszToken;
	return FALSE;
}

/*
	Function: MIMEBodyPart::ParseContentTransferEncoding
	Description:
		Parse a Content-Transfer-Encoding header.  Sets the
		m_pszEncoding member to a string describing the encoding.
*/
BOOL MIMEBodyPart::ParseContentTransferEncoding(char * pszHeader)
{
	ASSERT(pszHeader != NULL);

	// Allocate a token buffer.  The maximum token length could
	// be the size of the header minus the header name.  Allocate
	// one the size of the whole header, which is sure to be enough.
	int nTokenLen = strlen(pszHeader);
	char * pszToken = new char[nTokenLen];
	int iToken;
	MIMEParamTuple tuple;

	// Move up to the colon separating the header name from
	// the header body.
	while (*pszHeader != ':' && *pszHeader != '\0')
		pszHeader++;
	if (*pszHeader == '\0') goto error;
	// Move past the colon.
	pszHeader++;

	// Get the encoding.
	do {
		pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
			&iToken);
	} while (iToken == TOKEN_COMMENT);
	if (iToken != TOKEN_MIMETOKEN)
		goto error;
	m_pszEncoding = DuplicateString(pszToken);

	// See if we have any parameters.
	do {
		pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
			&iToken);
	} while (iToken == TOKEN_COMMENT);
	while (iToken == TOKEN_TSPECIAL && *pszToken == ';') {
		// Yes, we have a parameter.
		// Parse a token for the parameter name.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
		if (iToken != TOKEN_MIMETOKEN)
			goto error;
		tuple.Param(pszToken);
		// Get the equals sign.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
		if (iToken != TOKEN_TSPECIAL || *pszToken != '=')
			goto error;
		// Get the parameter value.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
		if (iToken != TOKEN_MIMETOKEN && iToken != TOKEN_QSTRING)
			goto error;
		tuple.Value(pszToken);
		// Add the tuple to the list of parameters.
		m_pParamList->Add(&tuple);
		// See if have another following ';'.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
	}

	// Okay, we should be at the end.
	delete[] pszToken;
	if (iToken != TOKEN_END)
		return FALSE;
	else
		return MIME_TRUE;

error:
	delete[] pszToken;
	return FALSE;
}

BOOL MIMEBodyPart::ParseContentDisposition(char * pszHeader)
{
	ASSERT(pszHeader != NULL);

	// Allocate a token buffer.  The maximum token length could
	// be the size of the header minus the header name.  Allocate
	// one the size of the whole header, which is sure to be enough.
	int nTokenLen = strlen(pszHeader);
	char * pszToken = new char[nTokenLen];
	int iToken;
	MIMEParamTuple tuple;

	// Move up to the colon separating the header name from
	// the header body.
	while (*pszHeader != ':' && *pszHeader != '\0')
		pszHeader++;
	if (*pszHeader == '\0') goto error;
	// Move past the colon.
	pszHeader++;

	// Get the disposition.
	do {
		pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
			&iToken);
	} while (iToken == TOKEN_COMMENT);
	if (iToken != TOKEN_MIMETOKEN)
		goto error;

	m_pszDisposition = DuplicateString(pszToken);

	// See if we have any parameters.
	do {
		pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
			&iToken);
	} while (iToken == TOKEN_COMMENT);
	while (iToken == TOKEN_TSPECIAL && *pszToken == ';') {
		// Yes, we have a parameter.
		// Parse a token for the parameter name.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
		if (iToken != TOKEN_MIMETOKEN)
			goto error;
		tuple.Param(pszToken);
		// Get the equals sign.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
		if (iToken != TOKEN_TSPECIAL || *pszToken != '=')
			goto error;
		// Get the parameter value.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
		if (iToken != TOKEN_MIMETOKEN && iToken != TOKEN_QSTRING)
			goto error;
		tuple.Value(pszToken);
		// Add the tuple to the list of parameters.
		m_pParamList->Add(&tuple);
		// See if have another following ';'.
		do {
			pszHeader = GetToken(MIME_TRUE, pszHeader, pszToken, nTokenLen,
				&iToken);
		} while (iToken == TOKEN_COMMENT);
	}

	// Okay, we should be at the end.
	delete[] pszToken;
	if (iToken != TOKEN_END)
		return FALSE;
	else
		return MIME_TRUE;

error:
	delete[] pszToken;
	return FALSE;
}

/*
	Function: MIMEBodyPart::FirstLine
	Description:
		Returns the first line of text in the body part.  The \n
		is stripped from the text.  Simply resets the current pointer
        to the first byte of the body part and then calls NextLine.
*/
int MIMEBodyPart::FirstLine(ZString & szLine, ostream & stm)
{
	m_lCurrent = m_lStart;
	return NextLine(szLine, stm);
}

/*
	Function: MIMEBodyPart::NextLine
	Description:
		Returns the next line of text in the body part.  The \n
		is stripped from the text.  Returns the number of characters
		transferred to the buffer, or -1 if there are no more lines
		of text.
*/
int MIMEBodyPart::NextLine(ZString & szLine, ostream &)
{
	szLine = "";
	if(m_lCurrent >= m_lEnd)
		return -1;

	while (m_lCurrent < m_lEnd)
	{
		int c = m_filemap[m_lCurrent++];
		if (c < 0 && c >= CMM_ERROR_MIN) {
			return -1;
		}
		if(c == '\n')
			break;
		szLine += c;
	}
	return szLine.length();
}

MIMEBodyPart* MIMEBodyPart::LastChild (void)
{
	MIMEBodyPart *pTemp=NULL, *pCurrentPart=m_pFirstChild;
	for( ; pCurrentPart; pCurrentPart = pCurrentPart->Sibling() )
		pTemp = pCurrentPart;
	return pTemp;
}

char * MIMEBodyPart::DuplicateString (const char * pszString)
{
	char * pszDuplicate = new char[strlen(pszString)+1];
	strcpy(pszDuplicate, pszString);
	return pszDuplicate;
}

BOOL MIMEBodyPart::IsHTML ()
{
	return (strcasecmp(m_pszType, "text") == 0 &&
		strcasecmp(m_pszSubType, "html") == 0);
}


/*********************************************************************
	Class: NewsMIME
*********************************************************************/

/*
	Function: NewsMIME::Parse
	Description:
		Parse the given filename.  Returns MIME_TRUE if the parse was
		successful or FALSE if an error occurred.
*/
BOOL NewsMIME::Parse(ostream & stm, const char * pFileName)
{
	if (m_pBodyParts != NULL) return FALSE;

	m_pBodyParts = new MIMEBodyPart(NULL, pFileName, 0, -1);
	if (!(m_pBodyParts->Open())) return FALSE;
	if (!(m_pBodyParts->Parse(stm, &m_bMultipart))) return FALSE;
	m_pMessageID = m_pBodyParts->MessageID();
	m_pBodyParts->Close();

    return MIME_TRUE;
}

BOOL NewsMIME::ParseHead(ostream & stm, char * pFileName)
{
	if (m_pBodyParts != NULL) return FALSE;

	m_pBodyParts = new MIMEBodyPart(NULL, pFileName, 0, -1);
	if (!(m_pBodyParts->Open())) return FALSE;
	if (!(m_pBodyParts->ParseHead(stm, &m_bMultipart))) return FALSE;
	m_pMessageID = m_pBodyParts->MessageID();
	m_pBodyParts->Close();

    return MIME_TRUE;
}

/*
	Function: NewsMIME::FirstPart
	Description:
		Moves to the first "viewable" body part in the tree,
		skipping over multipart nodes in favor of their
		children, as necessary.
*/
MIMEBodyPart *	NewsMIME::FirstPart(BOOL bTop/*=FALSE*/)
{
	m_pCurrentPart = m_pBodyParts;
	if (!bTop)
		while (m_pCurrentPart->FirstChild() != NULL)
			m_pCurrentPart = m_pCurrentPart->FirstChild();
	return m_pCurrentPart;
}

/*
	Function: NewsMIME::NextPart
	Description:
		Move to the next node in the MIME body part tree.  Returns
		the address of the next MIMEBodyPart object in the tree
		or NULL if there are no more nodes in the tree.
*/
MIMEBodyPart *	NewsMIME::NextPart(BOOL bTopChild/*=FALSE*/)
{
	// See if we can move sideways in the tree.
	MIMEBodyPart * pTemp = m_pCurrentPart;
	while (pTemp != NULL && pTemp->Sibling() == NULL) {
		// If not, then move up until we can move sideways.
		pTemp = pTemp->Parent();
	}
	// See if we've gone through all the nodes.
	if (!pTemp)
	{
		pTemp = m_pCurrentPart;
		if (pTemp->FirstChild())
			pTemp = pTemp->FirstChild();
		else
			return m_pCurrentPart = NULL;
	}
	else
		pTemp = pTemp->Sibling();

	if (!bTopChild)
		// If the node we found is a multipart node, move down as
		// far as possible.
		while (pTemp->FirstChild() != NULL)
			pTemp = pTemp->FirstChild();

	// Okay, we're there.
	return m_pCurrentPart = pTemp;
}

MIMEBodyPart* NewsMIME::GetBodyPart (ostream&, int nPart)
{
	MIMEBodyPart* pCurrentPart;
	for( pCurrentPart = FirstPart(TRUE);
		pCurrentPart; 
		pCurrentPart = NextPart(TRUE) )
	{
		if (pCurrentPart->PartNo() == nPart)
			break;
	}
	return m_pCurrentPart = pCurrentPart;
}
