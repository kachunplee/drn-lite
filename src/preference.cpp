#include <stdio.h>
#include "def.h"
#include "string.h"
#include "userinfo.h"
#include "preference.h"

Preference::Preference (NewsOptions opt, BOOL bUpdPref, BOOL BUpdQuote,
	BOOL bUpdSig)
{
	m_NewsOptions = opt;
	m_bUpdPref = bUpdPref;
	m_bUpdQuote = bUpdQuote;
	m_bUpdSig = bUpdSig;

	m_nPageLen = m_nDefPage = DEF_PAGE;
	m_nPageBeg = DEF_BEGIN;
	m_nFindOff = DEF_FINDOFF;
	m_cSortType = m_cDefSort = DEF_SORT;
	m_bBinCol = m_bDefBinCol = FALSE;
//?	m_bBinary = m_bStgOK = FALSE;
}

void Preference::Init (PCGI pcgi)
{
	if(pCGI && (m_bUpdPref || m_bUpdQuote || m_bUpdSig))
	{
		if(m_bUpdPref)
		{
			ReadOpenQuote();
			ReadSignature();
		}
		else if(m_bUpdQuote)
		{
			ReadPrefence();
			ReadSignature();
		}
		else if(m_bUpdSig)
		{
			ReadPrefence();
			ReadOpenQuote();
		}
	}
	else
	{
		ReadPrefence();
		ReadOpenQuote();
		ReadSignature();
	}
}

ostream& operator << (ostream& stm, Preference& pref)
{
	// read the user preference page to find the default sorting option
	//	and display page length
	ZString filename = USERDIR;
	filename += gUserInfo.GetUserName();
	filename += "/preference";
	Zifstream prefFile(filename.chars(), ios::nocreate|ios::in);
	if(prefFile && prefFile.good())
	{
		// Scan the preference file to look for sort type & page length
		char * q;
		do
		{
			q = prefFile.GetLine();
			if(*q == 0 || *q == '\r')
				break;
			
			if(strncasecmp(q, "sort=", 5) == 0)
			{
				m_cDefSort = q[5];
				//
				// To remain backward compatibility
				//
				switch(m_cDefSort)
				{
				case SORT_X_DATE:
					m_cDefSort = SORT_R_SUM_DATE;
					break;
				case SORT_SUBJECT:
					m_cDefSort = SORT_SUMMARY;
					break;
				case SORT_AUTHOR:
					m_cDefSort = SORT_SUM_AUTHOR;
					break;
				}
				m_cSortType = m_cDefSort;
			}
			else if(strncasecmp(q, "pagelength=", 11) == 0)
				m_nPageLen = m_nDefPage = atoi(q + 11);
			else if(strncasecmp(q, "bincol=", 7) == 0)
				m_bBinCol = m_bDefBinCol = (strncasecmp(q+7, "yes", 3) == 0);
		} while(prefFile.NextLine());
	}

}

void Preference::ReadPreference ()
{
}

void Preference::ReadOpenQuote ()
{
}

void Preference::ReadSignature ()
{
}
