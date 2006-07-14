SUBDIRS	= video
TARGETS	= GServer
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -ansi
LDFLAGSF= -l System  -l GL
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

GServer_OBJS = gserver.o
GServer_CFLAGS = $(CFLAGSF) $(CFLAGS)
GServer: $(GServer_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o GServer $(GServer_OBJS)

gserver.o: gserver.c
	$(CC) $(GServer_CFLAGS) -c gserver.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) $(GServer_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) $(GServer_OBJS)
	$(RM) $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 GServer $(DESTDIR)$(BINDIR)/GServer

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) $(DESTDIR)$(BINDIR)/GServer

.PHONY: all subdirs clean distclean install uninstall
