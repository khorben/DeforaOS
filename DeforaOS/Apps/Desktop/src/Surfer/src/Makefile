SUBDIRS	= common
TARGETS	= download surfer
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGS	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

download_OBJS = download.o
download_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D WITH_WEBKIT $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libSystem gtk+-2.0 webkit-1.0`
download_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libSystem gtk+-2.0 webkit-1.0`

download: $(download_OBJS)
	$(CC) -o download $(download_OBJS) $(download_LDFLAGS)

surfer_OBJS = surfer.o ghtml.o callbacks.o main.o
surfer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D WITH_WEBKIT $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libSystem libDesktop webkit-1.0`
surfer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libSystem libDesktop webkit-1.0`

surfer: $(surfer_OBJS)
	$(CC) -o surfer $(surfer_OBJS) $(surfer_LDFLAGS)

download.o: download.c download.h ../config.h
	$(CC) -D WITH_MAIN $(download_CFLAGS) -c download.c

surfer.o: surfer.c callbacks.h download.c download.h surfer.h ../config.h
	$(CC) $(surfer_CFLAGS) -c surfer.c

ghtml.o: ghtml.c ghtml.h ghtml-gtkhtml.c ghtml-gtkmozembed.c ghtml-gtktextview.c ghtml-webkit.c common/conn.c common/history.c common/url.c ../config.h
	$(CC) $(surfer_CFLAGS) -c ghtml.c

callbacks.o: callbacks.c surfer.h ghtml.h callbacks.h ../config.h
	$(CC) $(surfer_CFLAGS) -c callbacks.c

main.o: main.c surfer.h ../config.h
	$(CC) $(surfer_CFLAGS) -c main.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(download_OBJS) $(surfer_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(download_OBJS) $(surfer_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- download $(DESTDIR)$(BINDIR)/download
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- surfer $(DESTDIR)$(BINDIR)/surfer

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(BINDIR)/download
	$(RM) -- $(DESTDIR)$(BINDIR)/surfer

.PHONY: all subdirs clean distclean install uninstall
