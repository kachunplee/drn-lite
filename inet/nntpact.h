#ifndef __NNTPACT_H__
#define __NNTPACT_H__

class CNNTP;
class NNTPActive
{
public:
	NNTPActive();
	~NNTPActive();
	char * GetLine();

protected:
	CNNTP	* m_pNNTP;
};

#endif // __NNTPACT_H__
