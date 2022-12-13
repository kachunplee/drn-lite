#ifndef __PREFERENCE_H__
#define __PREFERENCE_H__

class Zifstream;
class Preference
{
protected:
	NewsOption&		m_opt;

	BOOL			m_bUpdPref;
	BOOL			m_bUpdQuote;
	BOOL			m_bUpdSig;

	int				m_nPageLen;
	int				m_nDefPage;
	int				m_nPageBeg;
	int				m_nFindOff;
	char			m_cSortType
	char			m_cDefSort;
	BOOL			m_bBinCol;
	BOOL			m_bDefBinCol;

public:
	Preference(NewsOptions &, BOOL, BOOL, BOOL);

	void Init(PCGI);

	friend ostream& operator <<(ostream&, Preference&);

	void ReadPreference();
	void ReadOpenQuote();
	void ReadSignature();
};

#endif //__PREFERENCE_H__
