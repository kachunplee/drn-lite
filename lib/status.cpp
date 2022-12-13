#include <stdarg.h>

#include "def.h"
#include "status.h"

const char begJavaScript [] = "<script language=javaScript>\n";
const char endScript [] = "</script>";

Status::~Status ()
{
	SetDefault();
}

void
Status::SetDefault ()
{
	if(!m_bChg)
		return;

	if(m_pstm == NULL)
		m_pstm = m_pstmsave;

	if(m_pstm == NULL)
		return;

	*m_pstm << begJavaScript
		<< "status = defaultStatus; " << endl
		<< endScript;
	m_bChg = false;
}

void
Status::operator () (const char * fmt, ...)
{
	if(m_pstm == NULL)
		return;

	va_list args;
	va_start(args, fmt);
	*m_pstm << begJavaScript
		<< "status = \"";
	m_pstm->vform(fmt, args);
	*m_pstm << "\"; " << endl << endScript;
	m_bChg = true;
}
