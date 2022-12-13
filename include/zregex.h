#ifndef __ZREGEX_H__
#define __ZREGEX_H__

#include <sys/types.h>
#include <regex.h>

class Regex
{
private:
	Regex(const Regex &) {}			// no X(X&)
	void operator = (const Regex&) {} // no assignment
public:
	Regex (const char * pattern, int cflags = REG_EXTENDED)
	{ m_err = regcomp(&m_Reg, pattern, cflags); }

	int RegExec (const char * stg,
			size_t nmatch = 0, regmatch_t * pmatch = NULL,
			int eflags = 0) const
	{ return m_err ? m_err : regexec(&m_Reg, stg, nmatch, pmatch, eflags); }

	size_t RegError (int errcode, char * errbuf, size_t errbuf_size)
	{ return regerror(errcode, &m_Reg, errbuf, errbuf_size); }

protected:
	regex_t	m_Reg;
	int		m_err;
};

#endif //__ZREGEX_H__
