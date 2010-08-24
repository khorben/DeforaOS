SUBDIRS	= video
TARGETS	= GServer
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I $(PREFIX)/include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags glu`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= -lSystem `pkg-config --libs glu`
LDFLAGS	= -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

GServer_OBJS = gserver.o main.o
GServer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
GServer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

GServer: $(GServer_OBJS)
	$(CC) -o GServer $(GServer_OBJS) $(GServer_LDFLAGS)

gserver.o: gserver.c gserver.h
	$(CC) $(GServer_CFLAGS) -c gserver.c

main.o: main.c gserver.h
	$(CC) $(GServer_CFLAGS) -c main.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(GServer_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(GServer_OBJS)
	$(RM) -- $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean install uninstall
