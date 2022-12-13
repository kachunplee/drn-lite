#include "rtt.h"

RTT::RTT ()
	: m_rtt(0), m_srtt(0), m_rttdev(1.5), m_nxtrto(0)
{
}

void
RTT::reset ()
{
	m_nrexmt = 0;
}

int
RTT::start ()
{
	if(m_nrexmt > 0)
	{
		// Retransmission
		m_currto *= RTT_BACKOFF[m_nrexmt];
		return m_currto;
	}

	gettimeofday(&m_start, 0);
	if(m_nxtrto > 0)
	{
		// First transmission of a packet
		m_currto = m_nxtrto;
		m_nxtrto = 0;
		return m_currto;
	}

	// Calculate timeout based on current estimators
	//	smoothed RTT plus twice the deviation

	m_currto = (int) (m_srtt + (2.0 * m_rttdev) + 0.5);
	return (m_currto < RTT_RXTMIN) ? RTT_RXTMIN :
		( (m_currto > RTT_RXTMAX) ? RTT_RXTMAX : m_currto);
	
	
}

void
RTT::stop ()
{
	if(m_nrexmt > 0)
	{	// The response was for a packet that has been rexmit
		m_nxtrto = m_currto;
		return;
	}
	m_nxtrto = 0;

	gettimeofday(&m_stop, 0);
	double start = ((double) m_start.tv_sec) * 1000000.0
				+ m_start.tv_usec;
	double stop = ((double) m_stop.tv_sec) * 1000000.0
				+ m_stop.tv_usec;
	m_rtt = (stop - start) / 1000000.0;

	//
	double err = m_rtt - m_srtt;
	m_srtt += err / 8;

	if(err < 0.0) err = -err;	// |err|

	m_rttdev += (err - m_rttdev) / 4;
}

int
RTT::timeout ()
{
	stop();

	if(++m_nrexmt > RTT_MAXNREXMT)
		return -1;		// time to give up

	return 0;
}
