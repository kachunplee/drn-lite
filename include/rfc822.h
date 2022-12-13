#ifndef __RFC822_H__
#define __RFC822_H__

/*
	CONSTANTS
*/
#define MAX_DATE_LENGTH 40
#define MAX_MESSAGE_ID_LENGTH 80

/*
	RFC 822 Definitions
*/
#define IsCHAR(c)	(0 <= c && c <= 127)
#define IsALPHA(c)	((65 <= c && c <= 90) || (97 <= c && c <= 122))
#define IsDIGIT(c)	(48 <= c && c <= 57)
#define IsCTL(c)	((0 <= c && c <= 31) || (127 == c))
#define CR			((char)13)
#define LF			((char)10)
#define SPACE		((char)32)
#define HTAB		((char)9)
#define IsCR(c)		(c == CR)
#define IsLF(c)		(c == LF)
#define IsSPACE(c)	(c == SPACE)
#define IsLWSP(c)	(c == SPACE || c == HTAB)
extern const char szSpecials[];
extern const char szTSpecials[];
#define IsSPECIAL(c)	IsInSet(c, szSpecials)
#define IsTSPECIAL(c)	IsInSet(c, szTSpecials)

/*
	TOKEN VALUES
*/
#define TOKEN_ERROR		-1
#define TOKEN_ATOM		1
#define TOKEN_MIMETOKEN	2
#define TOKEN_QSTRING	3
#define TOKEN_DOMLIT	4
#define TOKEN_COMMENT	5
#define TOKEN_SPECIAL	6
#define TOKEN_TSPECIAL	7
#define TOKEN_CTL		8
#define TOKEN_END		9

/*
	PARSING FUNCTIONS
*/
BOOL IsInSet(const int c, const char * szSet);
char * SkipLWSP(char * pszBuffer);
char * GetToken(BOOL bMIME, char * pszHeader, char * pszBuffer,
	int nLen, int * piType);

#endif //__RFC822_H__
