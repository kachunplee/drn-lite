#ifndef __NEWSOPT_H__
#define  __NEWSOPT_H__

const int SORT_ARTNUM	= 'n';
const int SORT_R_ARTNUM	= 'm';		// IE5 URL search is case insenitive
const int SORT_X_ARTNUM	= 'N';		// to remain compatible

const int SORT_DATE		= 'd';
const int SORT_R_DATE	= 'e';		// IE5 URL seach is case insenitive
const int SORT_X_DATE	= 'D';		// to remain compativle

const int SORT_SUM_DATE		= 'f';
const int SORT_R_SUM_DATE	= 'g';

const int SORT_SUBJECT	= 's';
const int SORT_SUMMARY	= 'z';

const int SORT_AUTHOR	= 'a';
const int SORT_SUM_AUTHOR	= 'b';

const int SORT_THREAD	= 't';

const int DEF_SORT = SORT_R_SUM_DATE;
const int DEF_BEGIN = 1;
const int DEF_PAGE = 100;
const int DEF_FINDOFF = 10;
const int DEF_ATTSPLITSIZE = 7500;



class NewsOption
{
protected:
	int		m_nPageLen;
	int		m_nDefPage;
	int		m_nPageBeg;
	int		m_nFindOff;
	int		m_nAttSplitSize;
	int		m_nMinSplitSize;
	int		m_nMaxSplitSize;
	char	m_cSortType;
	char	m_cDefSort;
	SBOOL	m_bBinary;
	SBOOL	m_bBinCol;
	SBOOL	m_bDefBinCol;
	SBOOL	m_bNoCrossPost;
	SBOOL	m_bNoXArchive;
	SBOOL	m_bStgOK;
	ZString	m_stg;
	ZString	m_stgFromName;
	ZString	m_stgFromEmail;
	ZString	m_stgOrganization;
	ZString	m_stgReplyEmail;
	ZString	m_stgMailDomain;
	ZString	m_stgDrnServer;
	ZString	m_stgWWWRoot;
	ZString	m_stgDrnTmplDir;
	ZString	m_stgUserDir;
	ZString	m_stgDecodeDir;
	ZString	m_stgLockDecodeDir;
	ZString	m_stgTempDecodeDir;
	ZString	m_stgDrnCacheDir;
	ZString	m_stgDrnCacheFile;
	ZString	m_stgDrnCleanCache;

	ZString	m_stgNNTPservers;
	ZString	m_stgNNTPauths;

public:
	NewsOption(int nP = DEF_PAGE, int nB = DEF_BEGIN, char cS = DEF_SORT,
		BOOL bBinary = FALSE);
	void ReadPreference();
	friend ostream& operator <<(ostream&, NewsOption&);

	NewsOption& operator = (const NewsOption&);

	void SetOptions(const char *);
	ZString & GetStg();
	char * StripQuote(char *);


	int GetPageLen () const						{ return m_nPageLen; }
	void SetPageLen (int n = DEF_PAGE)
	{ if(m_nPageLen != n) { m_nPageLen = n; m_bStgOK = FALSE; } }

	int GetPageBeg () const						{ return m_nPageBeg; }
	void SetPageBeg (int n = DEF_BEGIN)
	{ if(m_nPageBeg != n) { m_nPageBeg = n; m_bStgOK = FALSE; } }

	int GetFindOff () const						{ return m_nFindOff; }
	void SetFindOff (int n = DEF_FINDOFF)
	{ if(m_nFindOff != n) { m_nFindOff = n; m_bStgOK = FALSE; } }

	char GetSortType () const					{ return m_cSortType; }
	void SetSortType (char c = DEF_SORT)
	{ if(m_cSortType != c) { m_cSortType = c; m_bStgOK = FALSE; } }

	BOOL IsBinary () const						{ return m_bBinary; }
	void SetBinary (BOOL b)
	{ if(m_bBinary != b) { m_bBinary = b; m_bStgOK = FALSE; } }

	BOOL IsBinCol () const						{ return m_bBinCol; }
	void SetBinCol (BOOL b)
	{ if(m_bBinCol != b) { m_bBinCol = b; m_bStgOK = FALSE; } }

	int GetAttSplitSize () const			{ return m_nAttSplitSize; }
	int GetMinSplitSize () const			{ return m_nMinSplitSize; }
	int GetMaxSplitSize () const			{ return m_nMaxSplitSize; }
	BOOL IsNoCrossPost () const				{ return m_bNoCrossPost; }
	BOOL IsNoXArchive () const				{ return m_bNoXArchive; }
	const char * GetReplyEmail () 			{ return m_stgReplyEmail.chars(); }
	const char * GetFromName ()				{ return m_stgFromName.chars(); }
	const char * GetFromEmail ()			{ return m_stgFromEmail.chars(); }
	const char * GetOrganization ()			{ return m_stgOrganization.chars(); }
	const char * GetMailDomain ()			{ return m_stgMailDomain.chars(); }
	const char * GetDrnServer ()			{ return m_stgDrnServer.chars(); }
	const char * GetWWWRoot ()				{ return m_stgWWWRoot.chars(); }
};

#endif //  __NEWSOPT_H__
