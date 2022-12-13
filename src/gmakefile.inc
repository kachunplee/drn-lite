SRC_DIR="../src"
INET_DIR="../inet"
LIB_DIR="../lib"

#
#
#
vpath %.cpp ../inet ../src ../lib
vpath %.c ../inet ../src ../lib

#
#
#
SRCS=
OBJS=
#
#
#
INET_SRCS =\
	$(INET_DIR)/cinet.cpp $(INET_DIR)/inettcp.cpp \
	$(INET_DIR)/clienttcp.cpp $(INET_DIR)/cnntp.cpp \

SRCS += $(INET_SRCS)

INET_OBJS =\
	cinet.o inettcp.o \
	clienttcp.o cnntp.o \

OBJS += $(INET_OBJS)

#
#
LIB_SRCS= \
	$(SRC_DIR)/newsmime.cpp $(SRC_DIR)/newslib.cpp \
	$(SRC_DIR)/inlib.cpp $(SRC_DIR)/htmllib.cpp $(SRC_DIR)/urllib.cpp \
	$(SRC_DIR)/rfc822.cpp $(SRC_DIR)/cgi.cpp $(SRC_DIR)/httpbox.cpp \
	$(SRC_DIR)/zlist.cpp $(SRC_DIR)/zfilemap.cpp \
	$(SRC_DIR)/dmsg.cpp $(SRC_DIR)/assert.cpp $(SRC_DIR)/version.cpp \
	$(SRC_DIR)/cleanup.cpp $(SRC_DIR)/misc.cpp $(SRC_DIR)/wildmat.c \
	$(SRC_DIR)/ngshare.cpp \
	$(SRC_DIR)/userinfo.cpp $(SRC_DIR)/conf.cpp \
	$(SRC_DIR)/artspool.cpp \
	$(LIB_DIR)/status.cpp \
	$(LIB_DIR)/zcgi.cpp $(LIB_DIR)/zrtn.cpp \
	$(LIB_DIR)/zio.cpp $(LIB_DIR)/zstg.cpp \
	$(INET_DIR)/nntpact.cpp $(INET_DIR)/nntpart.cpp $(INET_DIR)/nntpov.cpp \


SRCS += $(LIB_SRCS)

#	gethashpwd.o getmd5.o mkdigest.o \

LIB_OBJS= \
	newsmime.o newslib.o \
	inlib.o htmllib.o urllib.o \
	rfc822.o \
	zlist.o zfilemap.o \
	status.o \
	dmsg.o assert.o version.o \
	cleanup.o misc.o wildmat.o \
	ngshare.o \
	userinfo.o conf.o \
	artspool.o \
	nntpact.o nntpart.o nntpov.o \
	zio.o zstg.o \
	$(INET_OBJS)

OBJS += $(LIB_OBJS)

NEWS_SRCS= \
	$(SRC_DIR)/newsopt.cpp \
	$(SRC_DIR)/newsgrp.cpp $(SRC_DIR)/newsart.cpp \
	$(SRC_DIR)/newslist.cpp $(SRC_DIR)/newsmidx.cpp \
	$(SRC_DIR)/newslidx.cpp $(SRC_DIR)/newslover.cpp $(SRC_DIR)/newsthread.cpp \
	$(SRC_DIR)/newslsort.cpp $(SRC_DIR)/merge.c $(SRC_DIR)/newslartn.cpp \
	$(SRC_DIR)/newslsubj.cpp $(SRC_DIR)/newslsum.cpp \
	$(SRC_DIR)/newslauth.cpp $(SRC_DIR)/newsldate.cpp \
	$(SRC_DIR)/newssorts.cpp $(SRC_DIR)/newssauth.cpp $(SRC_DIR)/newssdate.cpp \
	$(SRC_DIR)/advert.cpp $(SRC_DIR)/tmplerr.cpp \
	build.cpp

SRCS += $(NEWS_SRCS)

NEWS_OBJS= \
	newsopt.o \
	newsgrp.o newsart.o \
	newslist.o newsmidx.o \
	newslidx.o newslover.o newsthread.o \
	newslsort.o merge.o newslartn.o newslsubj.o newslsum.o \
	newslauth.o newsldate.o newssorts.o newssauth.o newssdate.o \
	advert.o tmplerr.o \
	build.o

OBJS += $(NEWS_OBJS)


#
#
#
PROGS += wwwnews

SRCS += $(SRC_DIR)/wwwnews.cpp

WWWNEWS_OBJS= \
	wwwnews.o

OBJS += $(WWWNEWS_OBJS)

wwwnews: $(WWWNEWS_OBJS) $(NEWS_OBJS) $(LIB_OBJS)
	$(CPP) -o $@ $(LDFLAGS) $^  $(LIBS)

#
#
#
PROGS += wwwprev

SRCS += $(SRC_DIR)/wwwprev.cpp

WWWPREV_OBJS= \
	wwwprev.o newsmidx.o

OBJS += $(WWWPREV_OBJS)

wwwprev: $(WWWPREV_OBJS) $(INET_OBJS) \
	newsopt.o newslib.o \
	newslover.o nntpov.o \
	newslidx.o newssorts.o newslsort.o merge.o newslsubj.o newslsum.o \
	newslartn.o newslauth.o newsldate.o newssdate.o newssauth.o newsthread.o \
	cleanup.o zstg.o conf.o zio.o urllib.o htmllib.o inlib.o \
	userinfo.o tmplerr.o dmsg.o assert.o misc.o status.o
	$(CPP) -o $@ $(LDFLAGS) $^  $(LIBS)

#
#
#
PROGS += wwwnext

SRCS += $(SRC_DIR)/wwwnext.cpp

WWWNEXT_OBJS= \
	wwwnext.o newsmidx.o

OBJS += $(WWWNEXT_OBJS)

wwwnext: $(WWWNEXT_OBJS) $(INET_OBJS) \
	newsopt.o newslib.o \
	newslover.o nntpov.o \
	newslidx.o newssorts.o newslsort.o merge.o newslsubj.o newslsum.o \
	newslartn.o newslauth.o newsldate.o newssdate.o newssauth.o newsthread.o \
	cleanup.o zstg.o conf.o zio.o urllib.o htmllib.o inlib.o \
	userinfo.o tmplerr.o dmsg.o assert.o misc.o status.o
	$(CPP) -o $@ $(LDFLAGS) $^  $(LIBS)

#
#
#
PROGS += wwwheader

SRCS += $(SRC_DIR)/wwwheader.cpp \
	$(SRC_DIR)/newsheader.cpp

WWWHEADER_OBJS= \
	wwwheader.o newsheader.o

OBJS += $(WWWHEADER_OBJS)

wwwheader: $(WWWHEADER_OBJS) $(INET_OBJS) \
	dmsg.o assert.o \
	newsopt.o artspool.o nntpart.o \
	cleanup.o zstg.o conf.o zio.o urllib.o htmllib.o inlib.o \
	build.o version.o userinfo.o tmplerr.o
	$(CPP) -o $@ $(LDFLAGS) $^  $(LIBS)

#
#
#
PROGS += wwwpost

SRCS += $(SRC_DIR)/wwwpost.cpp \
	$(SRC_DIR)/newspost.cpp

WWWPOST_OBJS= \
	wwwpost.o newspost.o

OBJS += $(WWWPOST_OBJS)

wwwpost: $(WWWPOST_OBJS) $(INET_OBJS) \
	dmsg.o assert.o \
	newsopt.o artspool.o nntpart.o \
	cleanup.o zstg.o conf.o zio.o urllib.o htmllib.o inlib.o \
	build.o version.o userinfo.o tmplerr.o
	$(CPP) -o $@ $(LDFLAGS) $^  $(LIBS)

#
#
#
PROGS += wwwinews

SRCS += $(SRC_DIR)/wwwinews.cpp

WWWINEWS_OBJS= \
	wwwinews.o zcgi.o zrtn.o

OBJS += $(WWWINEWS_OBJS)

wwwinews: $(WWWINEWS_OBJS) \
	$(INET_OBJS) nntpact.o dmsg.o assert.o \
	status.o newsopt.o \
	cleanup.o zstg.o conf.o zio.o \
	urllib.o htmllib.o inlib.o \
	build.o version.o userinfo.o tmplerr.o
	$(CPP) -o $@ $(LDFLAGS) $^  $(LIBS)

#
#
#
PROGS += wwwlistgrp

SRCS += $(SRC_DIR)/wwwlistgrp.cpp $(SRC_DIR)/newssub.cpp

WWWLISTGRP_OBJS= \
	wwwlistgrp.o newssub.o

OBJS += $(WWWLISTGRP_OBJS)

wwwlistgrp: $(WWWLISTGRP_OBJS) \
	$(INET_OBJS) nntpact.o newsopt.o newslib.o \
	newsgrp.o newslover.o nntpov.o ngshare.o \
	newslidx.o newssorts.o newslsort.o merge.o newslsubj.o newslsum.o \
	newslartn.o newslauth.o newsldate.o newssdate.o newssauth.o newsthread.o \
	cleanup.o zstg.o conf.o zio.o urllib.o htmllib.o inlib.o zrtn.o zcgi.o \
	userinfo.o tmplerr.o dmsg.o assert.o misc.o status.o build.o version.o
	$(CPP) -o $@ $(LDFLAGS) $^  $(LIBS)

#
#
#
PROGS += wwwsubscribe

SRCS += $(SRC_DIR)/wwwsubscribe.cpp

WWWSUBSCRIBE_OBJS= \
	wwwsubscribe.o

OBJS += $(WWWSUBSCRIBE_OBJS)

wwwsubscribe: $(WWWSUBSCRIBE_OBJS) \
	newsopt.o newssub.o \
	cleanup.o zstg.o conf.o zio.o urllib.o htmllib.o inlib.o zrtn.o zcgi.o \
	userinfo.o tmplerr.o dmsg.o assert.o misc.o status.o build.o version.o
	$(CPP) -o $@ $(LDFLAGS) $^  $(LIBS)

#
#
#
DECODE_SRCS= \
	$(SRC_DIR)/newsdecode.cpp \
	$(SRC_DIR)/uudecode.cpp $(SRC_DIR)/base64.cpp $(SRC_DIR)/hexbin.cpp

SRCS += $(DECODE_SRCS)

DECODE_OBJS= \
	newsdecode.o \
	uudecode.o base64.o hexbin.o \
	cgi.o httpbox.o \
	newsopt.o zio.o tmplerr.o

OBJS += $(DECODE_OBJS)

#
#
#
PROGS += wwwdecode

SRCS += $(SRC_DIR)/wwwdecode.cpp

WWWDECODE_OBJS= \
	wwwdecode.o

OBJS += $(WWWDECODE_OBJS)

wwwdecode: $(WWWDECODE_OBJS) $(DECODE_OBJS) version.o build.o $(LIB_OBJS)
	$(CPP) -o $@ $(LDFLAGS) $^  $(LIBS)

#
#
#
PROGS += wwwmdecode

SRCS += $(SRC_DIR)/wwwmdecode.cpp

WWWMDECODE_OBJS= \
	wwwmdecode.o

OBJS += $(WWWMDECODE_OBJS)

wwwmdecode: $(WWWMDECODE_OBJS) $(DECODE_OBJS) version.o build.o $(LIB_OBJS)
	$(CPP) -o $@ $(LDFLAGS) $^  $(LIBS)

#
#
#
#PROGS += drnclean

SRCS += $(SRC_DIR)/drnclean.cpp

DRNCLEAN_OBJS= \
	drnclean.o

OBJS += $(DRNCLEAN_OBJS)

drnclean: $(DRNCLEAN_OBJS)
	$(CPP) -o $@ $(LDFLAGS) $^  $(LIBS)

#
#
#
PROGS += finish

SRCS += $(SRC_DIR)/finish.c

FINISH_OBJS= \
	finish.o

OBJS += $(FINISH_OBJS)

finish: $(FINISH_OBJS)
	$(CPP) -o $@ $(LDFLAGS) $^

#
#
#

progs: $(PROGS)

clean:
	rm -f $(OBJS) $(PROGS)

depend:
	makedepend -f .depend $(CFLAGS) $(SRCS)
	/usr/local/bin/fixdepend .depend

include .depend
