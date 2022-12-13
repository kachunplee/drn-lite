#ifndef __RTT_H__
#define __RTT_H__

#include <sys/time.h>

// Base on UNIX Network Programming by Richard Stevens

const int RTT_RXTMIN    = 2;
const int RTT_RXTMAX    = 120;
const int RTT_MAXNREXMT = 3;
const int RTT_BACKOFF [] = { 1, 2, 4, 8, 16 };

class RTT
{
public:
	RTT();

	void reset();
	int start();
	void stop();
	int timeout();

	bool IsReXmit () const	{ return m_nrexmt > 0; }
	
protected:
	float	m_rtt;		// most recent round-trip time (sec)
	float	m_srtt;		// smoothed round-trip time (SRTT) (sec)
	float	m_rttdev;	// smoothed mean deviation (sec)
	short	m_nrexmt;	// #times retrunsmitted
	short	m_currto;	// current retransmit timeout
	short	m_nxtrto;	// retransmit time for next packet

	struct timeval m_start;
	struct timeval m_stop;
};

#endif // __RTT_H__
