#ifndef __NEWSDECODE_H__
#define __NEWSDECODE_H__

#ifdef LINUX
#include <db1/ndbm.h>
#else
#include <ndbm.h>
#endif

#include "cgi.h"
#include "newslib.h"
#include "newsmime.h"

enum RESULT
{
	RESULT_ERR=0,
	RESULT_OK,
	RESULT_ERRMSG,
	RESULT_ERRASSERT,
	RESULT_NONE,
};

enum MULTI
{
	MULTI_NONE=0,
	MULTI_MIME,
	MULTI_UUE,
};


extern void CleanUp(int);

class ArticleSpool;
class Zifstream;

class NewsDecode
{
protected:
	ZString&		m_stgArticle;
	int				m_nDecodePart;
	NewsSubject		m_Subject;
	NewsMIME		m_NewsMIME;
	ZString			m_MessageID;
	ZString			m_stgHashName;
	ZString			m_stgGroup;
	ZString			m_dirGroup;
	ZString			m_stgSubject;
	ZString			m_DecodeName;
	Zifstream *		m_pfileOver;
	int				m_nPartNo;
	int				m_nTotalPart;
	ZString *		m_pSubjects;
	ZString *		m_pNames;
	ZString *		m_pArtNums;
	ZString *		m_pMessageIDs;
	int *			m_pArtSizes;
	FILE *			m_pFile;
	int				m_nMulti;
	int				m_nLockFile;
	int				m_nLockMsg;
	BOOL			m_bMulti;
	BOOL			m_bForceDecode;
	BOOL			m_bSaveCache;
	BOOL			m_bCache;

public:
	NewsDecode (ZString & stg, int nPart, BOOL bSave) :
		m_stgArticle(stg), m_nDecodePart(nPart),
		m_pSubjects(NULL), m_pNames(NULL), m_pArtNums(NULL),
		m_pMessageIDs(NULL), m_pArtSizes(NULL), 
		m_pFile(NULL), m_nMulti(0), m_bMulti(FALSE), m_bSaveCache(bSave)
		{
			m_bForceDecode = FALSE;
			if(nPart == -2)
				m_bForceDecode = TRUE;
			m_nLockFile = -1;
			m_nLockMsg = -1;
			m_bCache = FALSE;
			m_pfileOver = NULL;
		}
	~NewsDecode();

	int GetMulti ()						{ return m_nMulti; }
	void SetMulti (int n)				{ m_nMulti = n; }

	friend ostream& operator <<(ostream&, NewsDecode&);

	void DecodeMIME(ostream &);
	int Decode_bodypart(ostream& stm);
	int Decode_multipart(ostream& stm);
	int Decode_multifile(ostream& stm);
	int Get_multifile(ostream& stm);
	void Adjust_multifilearray(int nSize);
	BOOL InitMultiFileDecode(PCGI);
	void Display_missing(ostream& stm);
	int Process_encoding(ostream& stm);
	void Display_parts(ostream& stm);

	void DecodeUU(ostream &, ArticleSpool &);
	void MultiUUEDecode(ostream &);
	BOOL FindPart(ostream &, char *);
	ZString & Mangle(ostream &, ZString &);
	BOOL FindUUBegin(ostream &, const char *);
	ZString& GetNameFromSubject(ZString &);

	BOOL OpenWriteFile(ostream &);
	void CloseWriteFile();
	int DecodeWriteFile(ostream &);
	int MultiMIMEDecode(ostream &);

	DBM * OpenDB(ostream &, char *);
	BOOL CacheExist(ostream &);
	BOOL FindCache(ostream &);
	void UnLockFile(const char *, int *);
	void LockFile(ostream &, const char *, int *);
	void CreateCacheName(ostream &, const char *);
	void UpdateDB(ostream &);
	void StoreCacheFile(ostream &, int);

	void OutLocation(ostream &, const char *);
	BOOL OpenOver();
};

#endif //__NEWSDECODE_H__
