#ifndef __NNTPOV_H__
#define __NNTPOV_H__

class CNNTP;
class NNTPOverview
{
public:
	NNTPOverview(const char * group);
	~NNTPOverview();
	char * GetLine();

	int	GetNumArt () const			{ return m_numArt; }

protected:
	CNNTP	* m_pNNTP;
	int		m_minArt;
	int		m_maxArt;
	int		m_numArt;
};

#endif // __NNTPOV_H__
