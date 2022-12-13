#include "conf.h"

#define DRN_DIR "/var/local/drn/"

//
// URLs
//
const char * DRN_SERVER			= "http://tdrn.pathlink.com";
const char * DECODEDIR_SERVER	= "http://tdrn.pathlink.com";

//
const char ARTICLE_SERVER []	= "";
const char DECODE_SERVER []		= "";

//
const char * DEF_WWWROOT		= DRN_DIR "www";
const char * DEF_MAILDOMAIN		= "pathlink.com";
const char * DEF_DRNSERVER		= "http://tdrn.pathlink.com";
const char * DEF_ORGANIZATION	= "[http://www.pathlink.com]";

//
const char * DEF_SENDER			= "usenet@tdrn.pathlink.com";
const char * DEF_PATH			= "drn";

//
// Template and images
// will be rebuild in the NewsOptions when WWWRoot is read from drn conf file
//
const char * DRNTMPLDIR 		= DRN_DIR "www" DRN_TMPLDIR;
const char * USERDIR			= DRN_DIR "www" DRN_USERDIR;
const char * ETCDIR				= DRN_DIR "www" DRN_ETCDIR;


//
//
const char DRN_HELP []			= "/drn/drnhelp.htm";
const char PREF_HELP []			= "/drn/prefhelp.htm";

const char * TMPDIR 			= "/tmp";

//
#ifndef NEWS_BIN
const char NEWSBIN []			= "/drn-bin";
#else
const char NEWSBIN []			= NEWS_BIN;
#endif

const char LINKDECODEDIR []		= "/decoded";

//
// Server
//
const char * NNTPservers = "news";
const char * NNTPauths = "";

//
// Delays
//
const int ACT_DELAY = 60*30;	// sec - between reloading active
const int OV_DELAY = 60*15;		// sec - between reloading overview
const int OV_IDX_DELAY = 0;		// sec - time lag between OverView->Index

//
//
const char * ACTIVEDIR			= DRN_DIR "www/db/";

//
// Decode Cache
// will be rebuild in the NewsOptions when WWWRoot is read from drn conf file
//
const char * DECODEDIR			= DRN_DIR "www" DRN_DECODEDIR;
const char * LOCKDECODEDIR		= DRN_DIR "www" DRN_LOCKDECODEDIR;
const char * TEMPDECODEDIR		= DRN_DIR "www" DRN_TEMPDECODEDIR;
const char * DRNCACHEDIR		= DRN_DIR "www" DRN_CACHEDIR;
const char * DRNCACHEFILE		= DRN_DIR "www" DRN_CACHEFILE;
const char * DRNCLEANCACHE		= DRN_DIR "www" DRN_CLEANCACHE;

//
const char DEF_BODY_TAG []		= "<body bgcolor=#ffffff>";
const char NO_GROUP_MSG []		= "<h2>Cannot access Newsgroup.</h2>";

const char DRN_NOTICE [] = "<a href=\"http://newsadm.com\"><I><font size=-1>Copyright Newsadm News Service</font></I></a>";

//
// Overview Indexes
//
const char NEWSUPDATE []		= DRN_DIR "www" "/ov.update/";
const char OVERVIEWDIR[]		= DRN_DIR "www" "/ov.index";
const char NEWSINDEX []			= DRN_DIR "www" "/ov.index/";
