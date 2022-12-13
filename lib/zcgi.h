#ifndef __ZCGI_H__
#define __ZCGI_H__

#include <string>
#include <map.h>

typedef map<string, string> ParamMap;

class ZCGI
{
public:
	ZCGI();
	~ZCGI();

	bool ReadInput();
	const char * GetParam(const char *);
	const char * GetFParam(const char *);
	int GetNParam(const char *);

	ParamMap & GetMap ()				{ return m_param; }
	ParamMap & GetFMap ()				{ return m_fparam; }

	void DumpParam();

protected:
	bool ReadURLEncoded(int nContent);
	bool ReadFormData(int nContent, char * pBoundary);

	int GetLine(char *, int);

protected:
	ParamMap	m_param;
	ParamMap	m_fparam;
	map<string, int> m_nparam;
};

#endif // __ZCGI_H__
