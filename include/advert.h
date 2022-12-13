#ifndef __ADVERT_H__
#define __ADVERT_H__


#define ADWIDTH		350
#define ADHEIGHT	70
#define ADBORDER	0

struct AdElm
{
	const char* pszURL;
	const char* pszImage;
};


class AdvertBanner
{
protected:
	ZString&		m_stg;
	const char *	m_pURL;
	const char *	m_pImage;

public:
	AdvertBanner (ZString& stg) : m_stg(stg)
	{
		m_pURL = m_pImage = NULL;
	}

	void GetBannerPtrs(BOOL bSave = TRUE);
	friend ostream& operator <<(ostream&, AdvertBanner&);

	const char * GetURL ()						{ return m_pURL; }
	const char * GetImage ()					{ return m_pImage; }
};


#endif //__ADVERT_H__
