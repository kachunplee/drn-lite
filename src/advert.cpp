#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>

#include "def.h"
#include "advert.h"

//
const char ADVERTISE [] 		= "/tmp/ng_advertise";
const char ADLTADVERTISE [] 		= "/tmp/ng_aadvertise";

enum
{
	ADV_BEG = 0,
//	ADV_01 = ADV_BEG,
//	ADV_02,
	ADV_03 = ADV_BEG,
//	ADV_04,
	ADV_05,
	SP_01,
//	ISP_01,
	ISP_02,
//	ISP_03,
	ISP_04,
	ISP_05,
//	ISP_06,
	ISP_07,
	ISP_08,
//	ISP_09,
//	ISP_10,
//	ADV_01_1,
//	ADV_02_1,
	ADV_03_1,
//	ADV_04_1,
	ADV_05_1,
//	ISP_11,
	ISP_12,
//	ISP_13,
//	ISP_14,
	ISP_15,
	ISP_16,
	ISP_17,
	ISP_18,
	ISP_19,
//	ISP_20,
//	ADV_01_2,
//	ADV_02_2,
	ADV_03_2,
//	ADV_04_2,
	ADV_05_2,
	ISP_21,
//	ISP_22,
	ISP_23,
//	ISP_24,
	ISP_25,
//	ISP_26,
	ISP_27,
	ISP_28,
	ISP_29,
//	ISP_30,
//	ISP_31,
	ISP_32,
//	ISP_33,
	ADV_MAX,
	ADLT_ADV_MAX = ADV_MAX,
//	ADLT_ADV_01 = ADV_MAX,
//	ADLT_ADV_02,
//	ADLT_ADV_03,
//	ADLT_ADV_MAX,
};

//const char ADV_URL_01 [] = "http://www.hansenwholesale.com";
//const char ADV_GIF_01 [] = "/drnbanner/ad_webproducers.gif";

//const char ADV_URL_02 [] = "http://www.ld.net/?warren";
//const char ADV_GIF_02 [] = "/drnbanner/ad_warren.gif";

const char ADV_URL_03 [] = "http://ld.net/zippo";
const char ADV_GIF_03 [] = "/drnbanner/ad_cognigen.gif";

//const char ADV_URL_04 [] = "http://www.nomadnews.com";
//const char ADV_GIF_04 [] = "/drnbanner/ad_nomadnews.gif";

const char ADV_URL_05 [] = "http://extra.newsadm.com";
const char ADV_GIF_05 [] = "/drnbanner/drnguy.gif";

const char SP_URL_01 [] = "http://www.spamhippo.com";
const char SP_GIF_01 [] = "/drnbanner/spamhippo.gif";

//const char ISP_URL_01 [] = "http://www.abraxis.com";
//const char ISP_GIF_01 [] = "/drnbanner/i_abraxis.gif";

const char ISP_URL_02 [] = "http://www.dialdata.com.br";
const char ISP_GIF_02 [] = "/drnbanner/i_dialdata.gif";

//const char ISP_URL_03 [] = "http://www.front.net";
//const char ISP_GIF_03 [] = "/drnbanner/i_front.gif";

const char ISP_URL_04 [] = "http://www.iftech.net";
const char ISP_GIF_04 [] = "/drnbanner/i_iftech.gif";

const char ISP_URL_05 [] = "http://www.ims-1.com";
const char ISP_GIF_05 [] = "/drnbanner/i_ims-1.gif";

//const char ISP_URL_06 [] = "http://www.ka.net";
//const char ISP_GIF_06 [] = "/drnbanner/i_ka.gif";

const char ISP_URL_07 [] = "http://www.kemmunet.net.mt";
const char ISP_GIF_07 [] = "/drnbanner/i_kemmunet.gif";

const char ISP_URL_08 [] = "http://www.lainet.com";
const char ISP_GIF_08 [] = "/drnbanner/i_lainet.gif";

//const char ISP_URL_09 [] = "http://mpb.com";
//const char ISP_GIF_09 [] = "/drnbanner/i_mpb.gif";

//const char ISP_URL_10 [] = "http://www.net-k.com.br";
//const char ISP_GIF_10 [] = "/drnbanner/i_net-k.gif";

//const char ISP_URL_11 [] = "http://www.nwcomputing.com";
//const char ISP_GIF_11 [] = "/drnbanner/i_nwcomp.gif";

const char ISP_URL_12 [] = "http://www.orlinter.com";
const char ISP_GIF_12 [] = "/drnbanner/i_orlinter.gif";

//const char ISP_URL_13 [] = "http://www.pcmvisual.com";
//const char ISP_GIF_13 [] = "/drnbanner/i_pcmvisual.gif";

//const char ISP_URL_14 [] = "http://www.smartgate.com";
//const char ISP_GIF_14 [] = "/drnbanner/i_smartgate.gif";

const char ISP_URL_15 [] = "http://www.bunt.com";
const char ISP_GIF_15 [] = "/drnbanner/i_ritanet.gif";

const char ISP_URL_16 [] = "http://www.ea.net";
const char ISP_GIF_16 [] = "/drnbanner/i_eanet.gif";

const char ISP_URL_17 [] = "http://www.silyn.net";
const char ISP_GIF_17 [] = "/drnbanner/i_silyn.gif";

const char ISP_URL_18 [] = "http://www.townhall.com";
const char ISP_GIF_18 [] = "/drnbanner/i_townhall.gif";

const char ISP_URL_19 [] = "http://www.dasia.net";
const char ISP_GIF_19 [] = "/drnbanner/i_dasia.gif";

//const char ISP_URL_20 [] = "http://www.macconnect.com";
//const char ISP_GIF_20 [] = "/drnbanner/i_macconnect.gif";

const char ISP_URL_21 [] = "http://www.actden.com";
const char ISP_GIF_21 [] = "/drnbanner/i_actden.gif";

//const char ISP_URL_22 [] = "http://www.a-o.com";
//const char ISP_GIF_22 [] = "/drnbanner/i_a-o.gif";

const char ISP_URL_23 [] = "http://www.systec.com";
const char ISP_GIF_23 [] = "/drnbanner/i_systec.gif";

//const char ISP_URL_24 [] = "http://www.portup.com";
//const char ISP_GIF_24 [] = "/drnbanner/i_portup.gif";

const char ISP_URL_25 [] = "http://www.artnet.net";
const char ISP_GIF_25 [] = "/drnbanner/i_artnet.gif";

//const char ISP_URL_26 [] = "http://www.ultracom.net";
//const char ISP_GIF_26 [] = "/drnbanner/i_ultracom.gif";

const char ISP_URL_27 [] = "http://www.interquest.de";
const char ISP_GIF_27 [] = "/drnbanner/i_interquest.gif";

const char ISP_URL_28 [] = "http://www.conninc.com";
const char ISP_GIF_28 [] = "/drnbanner/i_conninc.gif";

const char ISP_URL_29 [] = "http://www.power-online.net";
const char ISP_GIF_29 [] = "/drnbanner/i_power-online.gif";

//const char ISP_URL_30 [] = "http://home.acadia.net";
//const char ISP_GIF_30 [] = "/drnbanner/i_acadia.gif";

//const char ISP_URL_31 [] = "http://www.amazon.com.br";
//const char ISP_GIF_31 [] = "/drnbanner/i_amazon.gif";

const char ISP_URL_32 [] = "http://www.opensite.com.br";
const char ISP_GIF_32 [] = "/drnbanner/i_opensite.gif";

//const char ISP_URL_33 [] = "http://www.millcomm.com";
//const char ISP_GIF_33 [] = "/drnbanner/i_millcomm.gif";

//const char ADLT_URL_01 [] = "http://www.summer-knight.com";
//const char ADLT_GIF_01 [] = "/drnbanner/ad_summer-knight.gif";

//const char ADLT_URL_02 [] = "http://www.fleshxxx.com";
//const char ADLT_GIF_02 [] = "/drnbanner/ad_fleshxxx.gif";

//const char ADLT_URL_03 [] = "http://rocknrollsex.com";
//const char ADLT_GIF_03 [] = "/drnbanner/ad_rocknrollsex.gif";

//const char ADLT_URL_01 [] = "http://www.telepath.com/~roblow";
//const char ADLT_GIF_01 [] = "/drnbanner/ad_classica.gif";

static AdElm adv_array[] = {
//	{	ADV_URL_01,		ADV_GIF_01	},
//	{	ADV_URL_02,		ADV_GIF_02	},
	{	ADV_URL_03,		ADV_GIF_03	},
//	{	ADV_URL_04,		ADV_GIF_04	},
	{	ADV_URL_05,		ADV_GIF_05	},
	{	SP_URL_01,		SP_GIF_01	}, 
//	{	ISP_URL_01,		ISP_GIF_01	}, 
	{	ISP_URL_02,		ISP_GIF_02	}, 
//	{	ISP_URL_03,		ISP_GIF_03	}, 
	{	ISP_URL_04,		ISP_GIF_04	}, 
	{	ISP_URL_05,		ISP_GIF_05	}, 
//	{	ISP_URL_06,		ISP_GIF_06	}, 
	{	ISP_URL_07,		ISP_GIF_07	}, 
	{	ISP_URL_08,		ISP_GIF_08	}, 
//	{	ISP_URL_09,		ISP_GIF_09	}, 
//	{	ISP_URL_10,		ISP_GIF_10	}, 
//	{	ADV_URL_01,		ADV_GIF_01	},
//	{	ADV_URL_02,		ADV_GIF_02	},
	{	ADV_URL_03,		ADV_GIF_03	},
//	{	ADV_URL_04,		ADV_GIF_04	},
	{	ADV_URL_05,		ADV_GIF_05	},
//	{	ISP_URL_11,		ISP_GIF_11	}, 
	{	ISP_URL_12,		ISP_GIF_12	}, 
//	{	ISP_URL_13,		ISP_GIF_13	}, 
//	{	ISP_URL_14,		ISP_GIF_14	}, 
	{	ISP_URL_15,		ISP_GIF_15	}, 
	{	ISP_URL_16,		ISP_GIF_16	}, 
	{	ISP_URL_17,		ISP_GIF_17	}, 
	{	ISP_URL_18,		ISP_GIF_18	}, 
	{	ISP_URL_19,		ISP_GIF_19	}, 
//	{	ISP_URL_20,		ISP_GIF_20	}, 
//	{	ADV_URL_01,		ADV_GIF_01	},
//	{	ADV_URL_02,		ADV_GIF_02	},
	{	ADV_URL_03,		ADV_GIF_03	},
//	{	ADV_URL_04,		ADV_GIF_04	},
	{	ADV_URL_05,		ADV_GIF_05	},
	{	ISP_URL_21,		ISP_GIF_21	}, 
//	{	ISP_URL_22,		ISP_GIF_22	}, 
	{	ISP_URL_23,		ISP_GIF_23	}, 
//	{	ISP_URL_24,		ISP_GIF_24	}, 
	{	ISP_URL_25,		ISP_GIF_25	}, 
//	{	ISP_URL_26,		ISP_GIF_26	}, 
	{	ISP_URL_27,		ISP_GIF_27	}, 
	{	ISP_URL_28,		ISP_GIF_28	}, 
	{	ISP_URL_29,		ISP_GIF_29	}, 
//	{	ISP_URL_30,		ISP_GIF_30	}, 
//	{	ISP_URL_31,		ISP_GIF_31	}, 
	{	ISP_URL_32,		ISP_GIF_32	}, 
//	{	ISP_URL_33,		ISP_GIF_33	}, 
};

static AdElm adlt_array[] = {
//	{	ADV_URL_01,		ADV_GIF_01	},
//	{	ADV_URL_02,		ADV_GIF_02	},
	{	ADV_URL_03,		ADV_GIF_03	},
//	{	ADV_URL_04,		ADV_GIF_04	},
	{	ADV_URL_05,		ADV_GIF_05	},
	{	SP_URL_01,		SP_GIF_01	}, 
//	{	ISP_URL_01,		ISP_GIF_01	}, 
	{	ISP_URL_02,		ISP_GIF_02	}, 
//	{	ISP_URL_03,		ISP_GIF_03	}, 
	{	ISP_URL_04,		ISP_GIF_04	}, 
	{	ISP_URL_05,		ISP_GIF_05	}, 
//	{	ISP_URL_06,		ISP_GIF_06	}, 
	{	ISP_URL_07,		ISP_GIF_07	}, 
	{	ISP_URL_08,		ISP_GIF_08	}, 
//	{	ISP_URL_09,		ISP_GIF_09	}, 
//	{	ISP_URL_10,		ISP_GIF_10	}, 
//	{	ADV_URL_01,		ADV_GIF_01	},
//	{	ADV_URL_02,		ADV_GIF_02	},
	{	ADV_URL_03,		ADV_GIF_03	},
//	{	ADV_URL_04,		ADV_GIF_04	},
	{	ADV_URL_05,		ADV_GIF_05	},
//	{	ISP_URL_11,		ISP_GIF_11	}, 
	{	ISP_URL_12,		ISP_GIF_12	}, 
//	{	ISP_URL_13,		ISP_GIF_13	}, 
//	{	ISP_URL_14,		ISP_GIF_14	}, 
	{	ISP_URL_15,		ISP_GIF_15	}, 
	{	ISP_URL_16,		ISP_GIF_16	}, 
	{	ISP_URL_17,		ISP_GIF_17	}, 
	{	ISP_URL_18,		ISP_GIF_18	}, 
	{	ISP_URL_19,		ISP_GIF_19	}, 
//	{	ISP_URL_20,		ISP_GIF_20	}, 
//	{	ADV_URL_01,		ADV_GIF_01	},
//	{	ADV_URL_02,		ADV_GIF_02	},
	{	ADV_URL_03,		ADV_GIF_03	},
//	{	ADV_URL_04,		ADV_GIF_04	},
	{	ADV_URL_05,		ADV_GIF_05	},
	{	ISP_URL_21,		ISP_GIF_21	}, 
//	{	ISP_URL_22,		ISP_GIF_22	}, 
	{	ISP_URL_23,		ISP_GIF_23	}, 
//	{	ISP_URL_24,		ISP_GIF_24	}, 
	{	ISP_URL_25,		ISP_GIF_25	}, 
//	{	ISP_URL_26,		ISP_GIF_26	}, 
	{	ISP_URL_27,		ISP_GIF_27	}, 
	{	ISP_URL_28,		ISP_GIF_28	}, 
	{	ISP_URL_29,		ISP_GIF_29	}, 
//	{	ISP_URL_30,		ISP_GIF_30	}, 
//	{	ISP_URL_31,		ISP_GIF_31	}, 
	{	ISP_URL_32,		ISP_GIF_32	}, 
//	{	ISP_URL_33,		ISP_GIF_33	}, 
//	{	ADLT_URL_01,	ADLT_GIF_01	}, 
//	{	ADLT_URL_02,	ADLT_GIF_02	}, 
//	{	ADLT_URL_03,	ADLT_GIF_03	}, 
};


ostream& operator << (ostream& stm, AdvertBanner& banner)
{
	banner.GetBannerPtrs();

	stm << "<a href=\"" << banner.m_pURL << "\" target=_top>";
	stm << "<img src=\"" << banner.m_pImage << "\" alt=\""
		<< banner.m_pURL+7 << "\" border=" << ADBORDER
		<< "></a><br>";
 	return stm;
}

void AdvertBanner::GetBannerPtrs (BOOL bSave /*= TRUE*/)
{
	BOOL bAdult = FALSE;
	int nMax = ADV_MAX;
	const char * p = m_stg;
	if((strncmp(p, "alt.binaries", 12) == 0) || (strncmp(p, "alt.sex", 7) == 0))
	{
		bAdult = TRUE;
		nMax = ADLT_ADV_MAX;
	}
	char buf[6];
#ifdef FREEBSD
	int fd = open(bAdult?ADLTADVERTISE:ADVERTISE, O_RDWR|O_CREAT|O_EXLOCK,
			0644);
#else
	int fd = open(bAdult?ADLTADVERTISE:ADVERTISE, O_RDWR|O_CREAT,
			0644);
#endif
	int nDisp = ADV_BEG;
	int nSave = nDisp+1;
	int nRead;
	if(fd >= 0)
	{
		//flock(fd, LOCK_EX);
		if((nRead = read(fd, buf, 4)) >= 0)
		{
			buf[nRead] = 0;
			nDisp = atoi(buf);
			nSave = nDisp + 1;
			if(nSave >= nMax)
				nSave = 0;
			if(bSave)
			{
				sprintf(buf, "%04d\n", nSave);
				lseek(fd, 0, SEEK_SET);
				write(fd, buf, strlen(buf));
			}
		}
		//flock(fd, LOCK_UN);
		close(fd);
	}

	if(nDisp >= nMax)
		nDisp = ADV_BEG;

	m_pURL = bAdult ? adlt_array[nDisp].pszURL : adv_array[nDisp].pszURL;
	m_pImage = bAdult ? adlt_array[nDisp].pszImage : adv_array[nDisp].pszImage;
}
