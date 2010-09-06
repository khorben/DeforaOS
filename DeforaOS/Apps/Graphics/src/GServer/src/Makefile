SUBDIRS	= video
TARGETS	= ../data/GServer.h GServer
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -W `pkg-config --cflags gl`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= -lSystem `pkg-config --libs glu` -Wl,-export-dynamic
LDFLAGS	= -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

../data/GServer.h: ../data/GServer.interface
	./appbroker.sh "../data/GServer.h"

GServer_OBJS = gserver.o main.o
GServer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
GServer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

GServer: $(GServer_OBJS)
	$(CC) -o GServer $(GServer_OBJS) $(GServer_LDFLAGS)

gserver.o: gserver.c gserver.h ../data/GServer.h ../config.h
	$(CC) $(GServer_CFLAGS) -c gserver.c

main.o: main.c gserver.h
	$(CC) $(GServer_CFLAGS) -c main.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(../data/GServer.h_OBJS) $(GServer_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(../data/GServer.h_OBJS) $(GServer_OBJS)
	$(RM) -- $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean install uninstall
