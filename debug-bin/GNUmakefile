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
RELEASE?=0

#
#
#
NEWSBIN=drn-bin

DRN_DIR=/var/local/drn/www/$(NEWSBIN)/
DDN_DIR=/var/local/drn/www/$(NEWSBIN)/
BIN_DIR=/var/local/bin

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
CFLAGS +=-I. -I../include -I../inet -I../lib

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
#
install_drn: $(DRN_DIR)wwwnews \
	$(DRN_DIR)wwwprev $(DRN_DIR)wwwnext $(DRN_DIR)wwwheader \
	$(DRN_DIR)wwwpost $(DRN_DIR)wwwinews $(DRN_DIR)wwwlistgrp \
	$(DRN_DIR)wwwsubscribe

install_ddn: $(DDN_DIR)wwwdecode $(DDN_DIR)wwwmdecode

$(DRN_DIR)wwwnews $(DRN_DIR)wwwprev $(DRN_DIR)wwwnext \
  $(DRN_DIR)wwwheader $(DRN_DIR)wwwpost $(DRN_DIR)wwwinews \
  $(DRN_DIR)wwwlistgrp $(DRN_DIR)wwwsubscribe: \
	$(DRN_DIR)% : %
	install -C -o http -g WWW -s $(@F) $(DRN_DIR)

$(DDN_DIR)wwwdecode $(DDN_DIR)wwwmdecode: \
	$(DRN_DIR)% : %
	install -C -o http -g WWW -s $(@F) $(DDN_DIR)

include ../src/gmakefile.inc
