#ifndef __NEWSMIME_H__
#define __NEWSMIME_H__

#include "zlist.h"
#include "zfilemap.h"

/*
	Class: MIMEParamTuple
	Description:
		A simple class to store a 2-tuple of MIME parameter name and
		value strings.
*/
class MIMEParamTuple {
private:
	char *	m_pszParam;
	char *	m_pszValue;
public:
	MIMEParamTuple() : m_pszParam(NULL), m_pszValue(NULL) {};
	MIMEParamTuple(const char * pszParam, const char * pszValue)
		{ Param(pszParam); Value(pszValue); };
	MIMEParamTuple(const MIMEParamTuple&);
	~MIMEParamTuple();
	void Param(const char * pszParam);
	char * Param(void) { return m_pszParam; };
	void Value(const char * pszValue);
	char * Value(void) { return m_pszValue; };
	MIMEParamTuple& operator=(const MIMEParamTuple&);
};

/*
	Class: MIMEParamList
	Description:
		A simple list of parameter tuples.  Uses ZList as a base
		class and fills in the functionality appropriate for a list
		of MIMEParamTuples.
*/
class MIMEParamList : public ZList {
private:
	virtual int Compare(void * pvThing1, void * pvThing2);
    virtual void DeleteData(void * pvThing);
public:
	// Constructor and destructor.
	MIMEParamList() {};
	virtual ~MIMEParamList();

	// Manipulation functions.
	void Add(MIMEParamTuple * pTuple);
	void Insert(MIMEParamTuple * pTuple);
	MIMEParamTuple * GetParam(const char* pszString);
	MIMEParamTuple * First(void)
		{ return (MIMEParamTuple *) ZList::First(); };
	MIMEParamTuple * Next(void)
		{ return (MIMEParamTuple *) ZList::Next(); };
};

/*
	Class: MIMEBodyPart
	Description:
		Parses and stores data about individual MIME body parts.
		Handles multipart messages using a recursive parsing
		strategy.  When the parse is complete, a tree of
		MIMEBodyPart nodes will have been constructed which
		represents all the body parts in the message.
*/
class MIMEBodyPart {
private:
	BOOL				m_bEmpty;
	BOOL				m_bDigest;
	char *				m_pszFilename;
    ZFileMap			m_filemap;
	char *				m_pszType;
	char *  			m_pszSubType;
	MIMEParamList *		m_pParamList;
	char *				m_pszEncoding;
	char *				m_pszDisposition;
	long				m_lStart; // First char in the part
	long				m_lEnd;	// Last char in the part + 1
	long				m_lCurrent;
	long				m_lIndex;
	char *				m_pDispFileName;
	char *				m_pSubject;
	char *				m_pMessageID;
	// Relationships to other body parts in the message.
	// Note that we don't use a ZList-derived list of children
	// because we need to move more easily through the heirarchy
    // as we display the various body parts.
	MIMEBodyPart *		m_pParent;
	MIMEBodyPart *		m_pFirstChild;
	MIMEBodyPart *		m_pPrevSibling;
	MIMEBodyPart *		m_pSibling;
	int					m_nPart;

	// Private functions
	void SetDigest(BOOL bDigest=TRUE)	{ m_bDigest = bDigest; };
	void ParseSubject(ostream &);
	BOOL ParseHeaders(ostream &);
	BOOL ParseMultipart(ostream &);
	char * GetNextHeader(ostream &);
	long FindNewLine(ostream &);
    long StringSearch(char * pszString);
	BOOL ParseMIMEVersion(char * pszHeader);
	BOOL ParseContentType(char * pszHeader);
	BOOL ParseContentTransferEncoding(char * pszHeader);
	BOOL ParseContentDisposition(char * pszHeader);

public:
	MIMEBodyPart(MIMEBodyPart * pParent, const char *, long lStart, long lEnd);
	~MIMEBodyPart();

	BOOL Open(void);
	void Close(void);
	BOOL Parse(ostream &, BOOL *);
	BOOL ParseHead(ostream &, BOOL *);
	BOOL HeaderCompare(const char * pszHeader, char * pszHeaderName);

	BOOL IsEmpty(void) { return m_bEmpty; };
	char * Type(void) { return m_pszType; };
	char * SubType(void) { return m_pszSubType; };
	char * Encoding(void) { return m_pszEncoding; };
	char * Disposition(void) { return m_pszDisposition; };
	char * DispFileName(void) { return m_pDispFileName; };
	char * MessageID(void) { return m_pMessageID; };
	MIMEParamList *	ParamList(void) { return m_pParamList; };
	MIMEParamTuple * FirstParam(void)
		{ return m_pParamList->First(); };
	MIMEParamTuple * NextParam(void)
    	{ return m_pParamList->Next(); };

	int GetStart ()						{ return m_lStart; }
	int GetEnd ()						{ return m_lEnd; }

	int FirstLine(ZString & szLine, ostream &);
	int NextLine(ZString & szLine, ostream &);

	MIMEBodyPart * Parent(void) { return m_pParent; };
	MIMEBodyPart * FirstChild(void) { return m_pFirstChild; };
	MIMEBodyPart * LastChild(void);
	MIMEBodyPart * Sibling(void) { return m_pSibling; };
	MIMEBodyPart * PrevSibling(void) { return m_pPrevSibling; };
	int PartNo(void) { ASSERT(m_nPart>-1); return m_nPart; };

	char * DuplicateString(const char *);
	BOOL IsHTML();
};

/*
	Function: NewsMIME
	Description:
		An object which holds the top of the MIMEBodyPart tree
		and allows a user to navigate the tree.  MIME viewers should
		create one of these objects and access individual body
		parts in the tree using the navigation functions.
*/
class NewsMIME {
private:
	MIMEBodyPart *	m_pBodyParts;
	MIMEBodyPart *	m_pCurrentPart;
	char *			m_pMessageID;

protected:
	BOOL			m_bMIME;
	BOOL			m_bMultipart;
	

public:
	NewsMIME () : m_pBodyParts(NULL), m_pCurrentPart(NULL),
		m_bMIME(FALSE), m_bMultipart(FALSE)		{}
	~NewsMIME() { delete m_pBodyParts; };

	void SetMIME (BOOL b)						{ m_bMIME = b; } 
	BOOL IsMIME () const						{ return m_bMIME; } 
	BOOL IsMultipart () const					{ return m_bMultipart; } 

	BOOL Parse(ostream &, const char *);
	BOOL ParseHead(ostream &, char *);

	MIMEBodyPart * FirstPart(BOOL bTop=FALSE);
	MIMEBodyPart * NextPart(BOOL bTopChild=FALSE);
	MIMEBodyPart * CurrentPart(void)	{ return m_pCurrentPart; };
	void SetCurrent (MIMEBodyPart* pCurrent) { m_pCurrentPart = pCurrent; };
	char * MessageID ()					{ return m_pMessageID; }
	MIMEBodyPart * GetBodyPart(ostream& stm, int nPart);
};

#endif //__NEWSMIME_H__
