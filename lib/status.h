#ifndef __STATUS_H__
#define __STATUS_H__

#include <iostream.h>

class Status
{
public:
	Status (ostream * pstm) : m_pstm(pstm)
	{ m_bChg = FALSE; m_pstmsave = NULL; }
	~Status();

	void operator () (const char * fmt, ...);

	void SetDefault();
	ostream *	GetSTM () const				{ return m_pstm; }
	void PushSTM (ostream * pstm = NULL)
		{ m_pstmsave = m_pstm; m_pstm = pstm;}

protected:
	ostream *	m_pstm;
	ostream *	m_pstmsave;
	bool		m_bChg;
};

#endif // __STATUS_H__
