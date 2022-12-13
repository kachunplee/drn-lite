#ifndef __USERINFO_H__
#define __USERINFO_H__

class UserInfo
{
protected:
	ZString	m_UserName;
	ZString m_HostName;
	ZString m_HostAddr;

	ZString	m_HostDomain;
	int		m_nPortLimit;

	BYTE	m_nUserType;
	BOOL	m_bPasswd;

public:
	UserInfo ()
	{
		m_nPortLimit = 1;
		m_nUserType = 0; m_bPasswd = FALSE;
	}
	BOOL Init(ostream &, const char *, const char *, const char *);
	BOOL GetUserInfo(ZString &);
	BOOL GetHostInfo(ZString * pAccess = NULL);

	const char * GetUserName ()			{ return m_UserName; }
	const char * GetHostName ()			{ return m_HostName; }
	const char * GetHostAddr ()			{ return m_HostAddr; }
	const char * GetHostDomain ()			{ return m_HostDomain; }

	void SetUserType (int n)			{ m_nUserType = n; }
	BYTE GetUserType ()				{ return m_nUserType; }
};

extern UserInfo gUserInfo;

#endif // __USERINFO_H__
