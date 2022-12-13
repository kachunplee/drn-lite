.SUFFIXES: .o .cpp

.cpp.o:
	$(CPP) $(CFLAGS) -c $<

.c.o:
	$(CC) $(CFLAGS) -c $<

%.o: ../src/%.cpp
	$(CPP) $(CFLAGS) -c $<

%.o: ../src/%.c
	$(CC) $(CFLAGS) -c $<

#
RELEASE?=1

#
#
#
NEWSBIN=drn-bin
DIST=dist/drn

DRN_DIR=/var/local/drn/www/$(NEWSBIN)/
DDN_DIR=/var/local/drn/www/$(NEWSBIN)/
BIN_DIR=/var/local/bin

DIST_DIR=$(DIST)/www/$(NEWSBIN)/
DBIN_DIR=$(DIST)/bin/
SETUP_DIR=$(DIST)/www/drn-setup/

OPTION=-DSOLARIS -DSOL7 -DNEWS_BIN=\"/$(NEWSBIN)\"

#
LDFLAGS =
LIBS= -lsocket -lnsl

#
#
#
CC=gcc
CPP=g++
CFLAGS += -I. -I../inet -I../include -I../lib -I/usr/local/include/g++-3

#
#
#
ifeq "$(RELEASE)" "0"
CFLAGS += -g -O -Wall -DDEBUG $(OPTION)
else
CFLAGS += -g -O3 $(OPTION)
endif

#
#
#
make: progs

install_dist: $(DIST_DIR)wwwnews $(DIST_DIR)wwwprev $(DIST_DIR)wwwnext \
	$(DIST_DIR)wwwheader $(DIST_DIR)wwwpost $(DIST_DIR)wwwinews \
	$(DIST_DIR)wwwdecode $(DIST_DIR)wwwmdecode \
	$(DIST_DIR)wwwlistgrp $(DIST_DIR)wwwsubscribe \
	$(SETUP_DIR)finish

#	$(DBIN_DIR)drnclean

$(DIST_DIR)wwwnews $(DIST_DIR)wwwprev $(DIST_DIR)wwwnext \
  $(DIST_DIR)wwwheader $(DIST_DIR)wwwpost $(DIST_DIR)wwwinews \
  $(DIST_DIR)wwwdecode $(DIST_DIR)wwwmdecode \
  $(DIST_DIR)wwwlistgrp $(DIST_DIR)wwwsubscribe \
  : $(DIST_DIR)% : %
	install -o http -g WWW -s ${@F} $(DIST_DIR)

$(DBIN_DIR)drnclean \
  : $(DBIN_DIR)% : %
	install -o http -g WWW -s ${@F} $(DBIN_DIR)

$(SETUP_DIR)finish \
  : $(SETUP_DIR)% : %
	install -o root -g root -m 4555 -s ${@F} $(SETUP_DIR)

#
#
#
install_drn: $(DRN_DIR)wwwnews \
    $(DRN_DIR)wwwprev $(DRN_DIR)wwwnext $(DRN_DIR)wwwheader \
	$(DRN_DIR)wwwpost $(DRN_DIR)wwwinews \
	$(DRN_DIR)wwwlistgrp $(DRN_DIR)wwwsubscribe

install_ddn: $(DDN_DIR)wwwdecode $(DDN_DIR)wwwmdecode

$(DRN_DIR)wwwnews $(DRN_DIR)wwwprev $(DRN_DIR)wwwnext \
  $(DRN_DIR)wwwheader $(DRN_DIR)wwwpost $(DRN_DIR)wwwinews \
  $(DRN_DIR)wwwlistgrp $(DRN_DIR)wwwsubscribe \
	: $(DRN_DIR)% : %
	install -o http -g WWW -s ${@F} $(DDN_DIR) $(@F)

$(DDN_DIR)wwwdecode $(DDN_DIR)wwwmdecode \
	: $(DRN_DIR)% : %
	install -o http -g WWW -s ${@F} $(DDN_DIR) $(@F)

include ../src/gmakefile.inc
