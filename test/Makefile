.SUFFIXES: .o .cpp

.cpp.o:
	$(CPP) $(CFLAGS) -c $<

.c.o:
	$(CC) $(CFLAGS) -c $<

#
RELEASE?=0

COMPILER?=egcs

#
#
CFLAGS =
LFLAGS =
LIBS =

SRC_DIR=../src
#
#
#
.if $(COMPILER) == "egcs"
CC=/usr/local/bin/gcc
CPP=/usr/local/bin/eg++
CFLAGS +=-I. -I../include -I/usr/local/include/egcs/g++
LFLAGS = -g -L /usr/local/lib
LIBS +=
.else
CC=gcc
CPP=g++
CFLAGS +=-I. -I../include -fhandle-exceptions
LFLAGS = -g
.endif

#
#
#
.if $(RELEASE) == "0"
CFLAGS += -g -O -Wall -DDEBUG $(OPTION)
.else
CFLAGS += -O3 -Wall $(OPTION)
.endif

all: zstgfile

#
#
#
PROGS += zstgfile

SRCS += zstgfile.cpp $(SRC_DIR)/zstg.cpp 

ZSTGFILE_OBJS= \
	zstgfile.o zstg.o

OBJS += $(ZSTGFILE_OBJS)

zstgfile: $(ZSTGFILE_OBJS)
	$(CPP) -o $(.TARGET) $(.ALLSRC) $(LFLAGS) $(LIBS)

#
#
#
PROGS += zregtest

SRCS += zregtest.cpp $(SRC_DIR)/zstg.cpp 

ZREGTEST_OBJS= \
	zregtest.o zstg.o

OBJS += $(ZREGTEST_OBJS)

zregtest: $(ZREGTEST_OBJS)
	$(CPP) -o $(.TARGET) $(.ALLSRC) $(LFLAGS) $(LIBS)

#
#
#
PROGS += zstgtest

SRCS += zstgtest.cpp $(SRC_DIR)/zstg.cpp 

ZSTGTEST_OBJS= \
	zstgtest.o zstg.o

OBJS += $(ZSTGTEST_OBJS)

zstgtest: $(ZSTGTEST_OBJS)
	$(CPP) -o $(.TARGET) $(.ALLSRC) $(LFLAGS) $(LIBS)

#
#
#
PROGS += sstgtest

SRCS += sstgtest.cpp

sstgtest: sstgtest.cpp
	g++ -o $(.TARGET) $(.ALLSRC)

#
#
#
PROGS += multitest

SRCS += multitest.cpp $(SRC_DIR)/newslib.cpp 

MULTITEST_OBJS= \
	multitest.o newslib.o

OBJS += $(MULTITEST_OBJS)

multitest: $(MULTITEST_OBJS)
	$(CPP) -o $(.TARGET) $(.ALLSRC) $(LFLAGS) $(LIBS)

#
#
#

progs: $(PROGS)

clean:
	rm -f $(OBJS) $(PROGS)

depend:
	mkdep $(CFLAGS) $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
