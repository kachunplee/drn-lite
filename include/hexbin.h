#ifndef __HEXBIN_H__
#define __HEXBIN_H__

/* Mac time of 00:00:00 GMT, Jan 1, 1970 */
#define TIMEDIFF 0x7c25b080

#define DATABYTES 128

#define BYTEMASK 0xff
#define BYTEBIT 0x100
#define WORDMASK 0xffff
#define WORDBIT 0x10000

#define NAMEBYTES 63
#define H_NLENOFF 1
#define H_NAMEOFF 2

/* 65 <-> 80 is the FInfo structure */
#define H_TYPEOFF 65
#define H_AUTHOFF 69
#define H_FLAGOFF 73

#define H_LOCKOFF 81
#define H_DLENOFF 83
#define H_RLENOFF 87
#define H_CTIMOFF 91
#define H_MTIMOFF 95

#define H_OLD_DLENOFF 81
#define H_OLD_RLENOFF 85

#define F_BUNDLE 0x2000
#define F_LOCKED 0x8000

//
// Error definition
//
#define HB_EOF		-1
#define HB_ERR_OPEN	-2
#define HB_ERR_CRC	-3
#define HB_ERR_PART	-4

struct MacHeader
{
public:
	char		m_name[NAMEBYTES+1];
	char		m_type[4];
	char		m_author[4];
	short		m_flags;
	long		m_datalen;
	long		m_rsrclen;
	long		m_createtime;
	long		m_modifytime;
};

struct FileInfo
{
public:
	char		m_info[256];
	char		m_data[256];
	char		m_rsrc[256];
};

class ArticleSpool;
class ZHexBin
{
protected:
	ofstream			m_stm;
	struct MacHeader	m_MacHeader;	
	struct FileInfo		m_FileInfo;
	ZString				m_FileName;
	FILE *				m_pOutFile;
	ArticleSpool *		m_pArtSpool;
	int					m_nCompressed;		/* state variables */
	int					m_nQFormat;
	unsigned int		m_uCRC;
	BOOL				m_bMacBinary;
	BOOL				m_bPreBeta;
	char *				m_pBuffer;

	// For multi-parts
	int					m_nTotalPart;
	int					m_nCurPart;
	ZString *			m_pArtNums;
	const char *		m_pNewsName;

public:
	ZHexBin ()
	{
		m_uCRC = 0;
		m_bMacBinary = TRUE; m_bPreBeta = FALSE;
		m_pOutFile = NULL; m_pBuffer = NULL;
	}
	virtual ~ZHexBin()
	{ if(m_nCurPart>1) delete m_pArtSpool; CloseOutFile(); }

	FILE * GetOutFile ()				{ return m_pOutFile; }
	void SetQFormat (int n)				{ m_nQFormat = n; }
	int GetQFormat ()					{ return m_nQFormat; }
	BOOL IsMacBinary ()					{ return m_bMacBinary; }
	int GetCurPart ()					{ return m_nCurPart; }

	BOOL Init(ArticleSpool *, ZString &, int nTotalPart, ZString *, const char *);
	int Decode(ArticleSpool *);

	int FindHeader();
	BOOL SetupFiles(const char *);
	BOOL Open(const char *);
	BOOL OpenOutFile();
	void CloseOutFile();

	BOOL DoQHeader();
	BOOL DoOHeader(const char *);
	int ProcessForks();
	int DoQFork(char *, long);
	int DoOForks();
	int ForgeInfo();
	int VerifyCRC(unsigned int, unsigned int);
	void QInit();
	short Get2Q();
	long Get4Q();
	int GetQBuf(char *, int);
	int GetQByte();
	int GetQNoCRC();
	int GetQRaw();
	int Get6Bits();
	int OpenInFile();
	void CompQCRC(register unsigned int);
	long MakeFile(char *);
	int Comp2Bin(char *, FILE *);
	int Hex2Bin(char *, FILE *);
	int HexIt(char);
	void CompCCRC(unsigned char);
	void CompECRC(unsigned char);
	void Put2(char *, short);
	void Put4(char *, long);
};

#endif //__HEXBIN_H__
