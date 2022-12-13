#ifndef __NEWSLIB_H__
#define __NEWSLIB_H__

const int EXT_UNKNOWN = 0;
const int EXT_PHOTO = 1;
const int EXT_VIDEO = 2;
const int EXT_SOUND = 3;
const int EXT_BINARIES = 4;

class NewsSubject
{
protected:
	ZString	m_stg;

public:
	NewsSubject ()						{}
	NewsSubject (const char * p)		{ m_stg = p; }

	ZString & stg ()					{ return m_stg; }
	const char* chars() const			{ return m_stg; }
	operator const char *() const		{ return chars(); }

	BOOL IsMultiPart(int * pnNum = NULL, int * pnTotal = NULL,
		const char ** ppNum = NULL, const char ** ppTotal = NULL,
			int min = 0, int minTotal = 2);
	BOOL IsMultiPart(ZString &, int * pnTotal = NULL, int * pnNum = NULL);

	int GetFileExt(int * = NULL);
};

#endif // __NEWSLIB_H__
