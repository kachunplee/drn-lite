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

DIST_DIR=$(DIST)/www/$(NEWSBIN)/
DBIN_DIR=$(DIST)/bin/
SETUP_DIR=$(DIST)/www/drn-setup/

OPTION=-DFREEBSD -DNEWS_BIN=\"/$(NEWSBIN)\"

#
#
#
LIBS = -L../db -ldb_2.4.14 -lmd

#
#
#
CC=gcc
CPP=g++
CFLAGS +=-I. -I../inet -I../lib -I../include

#
#
#
ifeq "0" "$(RELEASE)"
CFLAGS += -g -O -Wall -DDEBUG $(OPTION)
else
CFLAGS += -g -O3 $(OPTION)
endif

make: progs

#
#
install_dist: $(DIST_DIR)wwwnews $(DIST_DIR)wwwprev $(DIST_DIR)wwwnext \
	$(DIST_DIR)wwwheader $(DIST_DIR)wwwpost $(DIST_DIR)wwwinews \
	$(DIST_DIR)wwwdecode $(DIST_DIR)wwwmdecode \
	$(DIST_DIR)wwwlistgrp $(DIST_DIR)wwwsubscribe \
	$(SETUP_DIR)finish

$(DIST_DIR)wwwnews $(DIST_DIR)wwwprev $(DIST_DIR)wwwnext \
  $(DIST_DIR)wwwheader $(DIST_DIR)wwwpost $(DIST_DIR)wwwinews \
  $(DIST_DIR)wwwdecode $(DIST_DIR)wwwmdecode \
  $(DIST_DIR)wwwlistgrp $(DIST_DIR)wwwsubscribe \
	: $(DIST_DIR)% : %
	install -C -o http -g WWW -s ${@F} $(DIST_DIR)

$(DBIN_DIR)drnclean: $(DBIN_DIR)% : %
	install -C -o http -g WWW -s ${@F} $(DBIN_DIR)

$(SETUP_DIR)finish: $(SETUP_DIR)% : %
	install -C -o 0 -g 0 -m 4555 -s ${@F} $(SETUP_DIR)

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
	install -C -o http -g WWW -s $(@F) $(DRN_DIR)

$(DDN_DIR)wwwdecode $(DDN_DIR)wwwmdecode \
	: $(DRN_DIR)% : %
	install -C -o http -g WWW -s $(@F) $(DDN_DIR)

include ../src/gmakefile.inc
