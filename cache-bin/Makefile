.SUFFIXES: .o .cpp

.cpp.o:
	$(CC) $(CFLAGS) -c $<

.c.o:
	gcc $(CFLAGS) -c $<

#
CC=g++

DEBUG?=1
#DEBUG?=0

BIN_DIR=/var/local/www/bin/
OPTION=

.if $(DEBUG) == "1"
CFLAGS=-g -Wall -DDEBUG $(OPTION)
#CFLAGS=-Wall -DDEBUG $(OPTION)
.else
CFLAGS=-O3 -Wall $(OPTION)
.endif

INC=
LIB_DIR=
LIB=-lcnews.a

checkcache:
	make $(BIN_DIR)checkcache

cleancache:
	make $(BIN_DIR)cleancache

createcache:
	make $(BIN_DIR)createcache

drnclean:
	make $(BIN_DIR)drnclean

listcache:
	make $(BIN_DIR)listcache

all: $(BIN_DIR)checkcache $(BIN_DIR)cleancache $(BIN_DIR)createcache \
	$(BIN_DIR)drnclean $(BIN_DIR)listcache


$(BIN_DIR)checkcache: checkcache.o
	$(CC) -o $@ checkcache.o

$(BIN_DIR)cleancache: cleancache.o
	$(CC) -o $@ cleancache.o

$(BIN_DIR)createcache: createcache.o
	$(CC) -o $@ createcache.o

$(BIN_DIR)drnclean: drnclean.o
	$(CC) -o $@ drnclean.o

$(BIN_DIR)listcache: listcache.o
	$(CC) -o $@ listcache.o


drnclean.o: drncache.h

createcache.o: drncache.h

listcache.o: drncache.h
