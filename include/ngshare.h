#ifndef __NGSHARE_H__
#define __NGSHARE_H__

class NNTPActive;
class NewsGroupShare
{
protected:
	int				m_nIdxLine;
	NNTPActive *	m_pActive;

	ZString			m_errmsg;

public:
	NewsGroupShare();
	~NewsGroupShare();

	BOOL BeginList();
	char * GetLine(ZString &);

	int GetGroupIndex(const char *);

#ifdef REGULAR
	int GetMinArtNum(const char *);
#endif

protected:
	BOOL Open();
	BOOL IsTimeUp();
	int FindGroup(const char *);
};

#endif // __NGSHARE_H__
