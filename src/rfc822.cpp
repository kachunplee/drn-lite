#include <time.h>

#include "def.h"

#include "rfc822.h"

/*********************************************************************
	Parsing Functions
*********************************************************************/

// Standard RFC 822 specials
const char szSpecials[] = "()<>@,;:\\\".[]";
// MIME (RFC 1521) tspecials
const char szTSpecials[] = "()<>@,;:\\\"/[]?=";

/*
	Function: IsInSet
	Description:
		Returns TRUE if the specified character is a part of the
		given character set.
*/
BOOL IsInSet(const int c, const char * szSet)
{
	int i;

	for (i = 0; szSet[i] != '\0'; i++)
		if (szSet[i] == c) return TRUE;
	return FALSE;
}

/*
	Function: SkipLWSP
	Description:
		Returns the first character of the specified string
		that is not a linear whitespace character.
*/
char * SkipLWSP(char * pszBuffer)
{
	while (IsLWSP(*pszBuffer))
		pszBuffer++;
	return pszBuffer;
}

/*
	Function: LexAtom
	Description:
		Gets an atom from the specified header.  Returns the
		first character that could not be consumed. The buffer length
		is specified in nLen and must be long enough to hold the
		entire token.
*/
char * LexAtom(char * pszHeader, char * pszBuffer, int nLen)
{
	int iHeader = 0;
	int iBuffer = 0;
	char c = pszHeader[iHeader++];
	while (!IsSPECIAL(c) && !IsSPACE(c) && !IsCTL(c) &&
		iBuffer < (nLen - 1)) {
		pszBuffer[iBuffer++] = c;
		c = pszHeader[iHeader++];
	}
	pszBuffer[iBuffer] = '\0';
	return pszHeader + iHeader - 1;
}

/*
	Function: LexQText
	Description:
		Gets quoted text from the specified header.  Returns the
		first character that could not be consumed. The buffer length
		is specified in nLen and must be long enough to hold the
		entire token.
*/
char * LexQText(char * pszHeader, char * pszBuffer, int nLen)
{
	int iHeader = 0;
	int iBuffer = 0;
	char c = pszHeader[iHeader++];
	while (c != '"' && iBuffer < (nLen - 1)) {
		// Skip over quoted pairs.
		if (c == '\\') {
			pszBuffer[iBuffer++] = pszHeader[iHeader++];
		}
		else {
			pszBuffer[iBuffer++] = c;
		}
		c = pszHeader[iHeader++];
	}
	pszBuffer[iBuffer] = '\0';
	return pszHeader + iHeader - 1;
}

/*
	Function: LexDText
	Description:
		Gets a domain literal from the specified header. Returns the
		first character that could not be consumed. The buffer length
		is specified in nLen and must be long enough to hold the
		entire token.
*/
char * LexDText(char * pszHeader, char * pszBuffer, int nLen)
{
	int iHeader = 0;
	int iBuffer = 0;
	char c = pszHeader[iHeader++];
	while (c != '[' && c != ']' && iBuffer < (nLen - 1)) {
		// Skip over quoted pairs.
		if (c == '\\') {
			pszBuffer[iBuffer++] = pszHeader[iHeader++];
		}
		else {
			pszBuffer[iBuffer++] = c;
		}
		c = pszHeader[iHeader++];
	}
	pszBuffer[iBuffer] = '\0';
	return pszHeader + iHeader - 1;
}

/*
	Function: LexCText
	Description:
		Gets comment text from the specified header.  Returns the
		first character that could not be consumed. The buffer length
		is specified in nLen and must be long enough to hold the
		entire token.
*/
char * LexCText(char * pszHeader, char * pszBuffer, int nLen)
{
	int iHeader = 0;
	int iBuffer = 0;
	int iNestLevel = 0;
	char c = pszHeader[iHeader++];
	while ((c != ')' || iNestLevel != 0) && iBuffer < (nLen - 1)) {
		// Skip over quoted pairs.
		if (c == '\\') {
			pszBuffer[iBuffer++] = pszHeader[iHeader++];
		}
		else {
			pszBuffer[iBuffer++] = c;
			if (c == '(')
				iNestLevel++;
			if (c == ')')
            	iNestLevel--;
		}
		c = pszHeader[iHeader++];
	}
	pszBuffer[iBuffer] = '\0';
	return pszHeader + iHeader - 1;
}

/*
	Function: LexMIMEToken
	Description:
		Gets MIME token from the specified header.  A MIME token
		is defined in RFC 1521 and is slightly different than an
		RFC 822 atom.  The function returns the first character in
        the stream that could not be consumed.
*/
char * LexMIMEToken(char * pszHeader, char * pszBuffer, int nLen)
{
	int iHeader = 0;
	int iBuffer = 0;
	char c = pszHeader[iHeader++];
	while (!IsTSPECIAL(c) && !IsSPACE(c) && !IsCTL(c) &&
		iBuffer < (nLen - 1)) {
		pszBuffer[iBuffer++] = c;
		c = pszHeader[iHeader++];
	}
	pszBuffer[iBuffer] = '\0';
	return pszHeader + iHeader - 1;
}

/*
	Function: GetToken
	Description:
		Gets an RFC 822 token of some sort from the buffer.  The
		buffer is filled with the token and the integer specified
		by piType is set to the token type.  The next character
		following the token is returned.  If an error occurs,
		*piType is set to TOKEN_ERROR and the function return value 
		indicates where the error occurred.  If bMIME is true, the
		function looks for MIME tokens rather than RFC 822 atoms.
		Note that the enclosing characters of quoted text,
		domain literals, and comments ( "\"[]()" ) is stripped
		from the token before it is returned.

		Note that pszBuffer must be non-NULL and nLen must 2 or more
		(2 for special or CTL and terminating null).
*/
char * GetToken(BOOL bMIME, char * pszHeader, char * pszBuffer,
	int nLen, int * piType)
{
	ASSERT(pszBuffer != NULL);
	ASSERT(nLen >= 2);

	// Skip over any leading white space characters.
	pszHeader = SkipLWSP(pszHeader);
	if (*pszHeader == '\0') {
    	// We're at the end of the string.
		*pszBuffer = '\0';
		*piType = TOKEN_END;
		return pszHeader;
	}
	else if (*pszHeader == '"') {
		// Parse some quoted text.
		pszHeader = LexQText(pszHeader + 1, pszBuffer, nLen);
		// See if we stopped because of terminating quote.
		if (*pszHeader == '"') {
			// Yes, so everything was okay.
			*piType = TOKEN_QSTRING;
			return pszHeader + 1;
		}
		else {
			// Nope.  Signal error.
			*piType = TOKEN_ERROR;
			return pszHeader;
		}
	}
	else if (*pszHeader == '[') {
		// Parse a domain literal.
		pszHeader = LexDText(pszHeader + 1, pszBuffer, nLen);
		// See if we stopped because of terminating ']'.
		if (*pszHeader == ']') {
			// Yes, so everything was okay.
			*piType = TOKEN_DOMLIT;
			return pszHeader + 1;
		}
		else {
			// Nope.  Signal error.
			*piType = TOKEN_ERROR;
			return pszHeader;
		}
	}
	else if (*pszHeader == '(') {
		// Parse a comment.
		pszHeader = LexCText(pszHeader + 1, pszBuffer, nLen);
		// See if we stopped because of terminating ')'.
		if (*pszHeader == ')') {
			// Yes, so everything was okay.
			*piType = TOKEN_DOMLIT;
			return pszHeader + 1;
		}
		else {
			// Nope.  Signal error.
			*piType = TOKEN_ERROR;
			return pszHeader;
		}
	}
	else if (IsCTL(*pszHeader)) {
		*piType = TOKEN_CTL;
		pszBuffer[0] = *pszHeader;
		pszBuffer[1] = '\0';
        return pszHeader + 1;
	}
	else if (bMIME) {
		// Parse MIME tokens and tspecials.
		if (IsTSPECIAL(*pszHeader)) {
			*piType = TOKEN_TSPECIAL;
			pszBuffer[0] = *pszHeader;
			pszBuffer[1] = '\0';
			return pszHeader + 1;
		}
		else {
			pszHeader = LexMIMEToken(pszHeader, pszBuffer, nLen);
			*piType = TOKEN_MIMETOKEN;
            return pszHeader;
		}
	}
	else {
		// Parse RFC 822 atoms and specials.
		if (IsSPECIAL(*pszHeader)) {
			*piType = TOKEN_SPECIAL;
			pszBuffer[0] = *pszHeader;
			pszBuffer[1] = '\0';
			return pszHeader + 1;
		}
		else {
			pszHeader = LexAtom(pszHeader, pszBuffer, nLen);
			*piType = TOKEN_ATOM;
            return pszHeader;
        }
	}
}

