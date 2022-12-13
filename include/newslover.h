#ifndef __NEWSLOVER_H__
#define __NEWSLOVER_H__

#include <sys/types.h>
#include <time.h>
#include "newslidx.h"

const int FIELD_ARTNO	= 1;
const int FIELD_SUBJ	= 2;
const int FIELD_FROM	= 3;
const int FIELD_DATE	= 4;
const int FIELD_MSGID	= 5;
const int FIELD_REF		= 6;
const int FIELD_BYTES	= 7;
const int FIELD_LINE	= 8;

//
// Use 21 bit for index -> 2M articles max
//	extend to 64 bit when there is 2M article groups.
// Upper 10 bit is used as needed different sorts.
//
const int IDX_BIT = 21;

const unsigned IDX_MASK = (1 << IDX_BIT)-1;
const unsigned IDX_SPC = (1 << (32-IDX_BIT))-1;

//
// Use 30 bit for size in byte
//
const int BYTE_BIT = 28;

const unsigned BYTE_MASK = (1<<BYTE_BIT)-1;

//
// One use is the top 11 bit of idx for num of part got and completed
//
const int PART_BIT = 10;
const int PART_MASK = (1<<10)-1;

// top 2 bit of byte for multi and complete
const int PART_MULTI    = 0x80000000;
const int PART_COMPLETE = 0x40000000;

struct IndexThg
{
	streamoff	off;
	ARTNUM		artnum;
	unsigned	idx;
	unsigned	bytes;
};

struct ArtNumIdx
{
	streamoff	off;
	ARTNUM		artnum;
};

//
// News List using overview file
//
class NewsListOver : public NewsListIndex
{
public:
	NewsListOver (const char * grp, const char * file,
		ostream * poutStat, int min = 0);
	~NewsListOver();

	int Fetch();
	int Find(ARTNUM);
	ARTNUM GetArtNum(int);
	unsigned GetSize(int);

	unsigned GetArtIdx(int);
	unsigned GetBytes(int);

	int Set(int nBeg, int nGet, int nDir = 1);

	BOOL Get(int idx, ZString &);

	void Bad (); 

protected:
	ZString		m_stgFileName;

	int			m_fd;

	Zifstream	m_fileOver;

#ifdef FREEBSD
	struct timespec	m_mtimeOver;
#else
	time_t		m_mtimeOver;
#endif
	size_t 		m_sizeIdx;

	int			m_nIndex;

	int			m_nArtNum;
	ArtNumIdx *		m_ArtNumIdx;
	
	IndexThg *		m_Thgs;
	IndexThg *		m_pThg;

protected:
	virtual const char * GetFileName () = 0;	// name of index file
												// make sure don't
												// something that
												// someone else is using

	size_t operator [] (int idx);

	virtual BOOL GetHeaders();
	virtual BOOL GetArtNumList();

	virtual BOOL BegOver () = 0;
	virtual BOOL SetOver (int, ssize_t, char *) = 0;
	virtual BOOL EndOver () = 0;

	virtual BOOL Sort () = 0;

	// Helps
	int Make();

	BOOL Save();
	BOOL WriteIndex();
	int	 SetArtNum();

	int	GetDelayTime(const char *);
	BOOL OpenOver();

	//
	// Some helpers for summary listings
	//
	ostream &OutSubjectSum (ostream & stm, int n,
	  const char * grp, const char * artno, const char * subj,
	  int pagelen, int iFile);
	ostream &OutURLSum (ostream & stm, int n,
	  const char * grp, const char * artno, int pagelen);
	int GetMultiSum (int);
};

#endif // __NEWSLOVER_H__
