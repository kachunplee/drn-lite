#ifndef __CONF_H__
#define __CONF_H__

#include <sys/types.h>

#define DRN_TMPLDIR "/html/template";
#define DRN_USERDIR "/user/";
#define DRN_ETCDIR "/etc/";
#define DRN_DECODEDIR "/decoded"
#define DRN_LOCKDECODEDIR "/decoded/lock/"
#define DRN_TEMPDECODEDIR "/decoded/temp/"
#define DRN_CACHEDIR "/dd.index/"
#define DRN_CACHEFILE "/dd.index/drncache"
#define DRN_CLEANCACHE "/bin/cleancache"

extern const char * DRN_SERVER;
extern const char * DECODEDIR_SERVER;
extern const char ARTICLE_SERVER[];
extern const char DECODE_SERVER[];

extern const char HOME_SERVER[];

extern const char * NNTPservers;
extern const char * NNTPauths;

extern const char * DEF_MAILDOMAIN;
extern const char * DEF_DRNSERVER;
extern const char * DEF_WWWROOT;
extern const char * DEF_ORGANIZATION;
extern const char * DEF_PATH;
extern const char * DEF_SENDER;

extern const char DRN_HELP[];
extern const char PREF_HELP[];

extern const char * DRNTMPLDIR;
extern const char * USERDIR;
extern const char * ETCDIR;
extern const char * DECODEDIR;
extern const char * LOCKDECODEDIR;
extern const char * TEMPDECODEDIR;
extern const char * DRNCACHEDIR;
extern const char * DRNCACHEFILE;
extern const char * DRNCLEANCACHE;
extern const char * ACTIVEDIR;

extern const char * TMPDIR;
extern const char OVERVIEWDIR[];
extern const char LINKDECODEDIR[];

extern const char NEWSUPDATE[];

extern const char NEWSLIST[];

extern const char USERPID[];

extern const char DBNAME[];

#ifndef NEWS_BIN
extern const char NEWSBIN[];
#else
extern const char NEWSBIN[];
#endif

extern const char USERBIN[];

extern const char USERINFODB[];
extern const char HOSTINFODB[];

extern const char NEWSINDEX[];

extern const char DEF_BODY_TAG[];

extern const char NO_GROUP_MSG[];
extern const char DRN_NOTICE[];

extern const int ACT_DELAY;
extern const int OV_DELAY;
extern const int OV_IDX_DELAY;

#endif // __CONF_H__
