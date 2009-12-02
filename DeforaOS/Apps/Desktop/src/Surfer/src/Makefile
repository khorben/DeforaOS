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


all: $(TARGETS)

download_OBJS = download.o
download_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags gtk+-2.0 gnet-2.0`
download_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs gtk+-2.0 gnet-2.0`

download: $(download_OBJS)
	$(CC) -o download $(download_OBJS) $(download_LDFLAGS)

surfer_OBJS = surfer.o ghtml.o callbacks.o main.o
surfer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D WITH_WEBKIT $(CFLAGSF) $(CFLAGS) `pkg-config --cflags gtk+-2.0 webkit-1.0`
surfer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem `pkg-config --libs gtk+-2.0 webkit-1.0`

surfer: $(surfer_OBJS)
	$(CC) -o surfer $(surfer_OBJS) $(surfer_LDFLAGS)

download.o: download.c ../config.h
	$(CC) $(download_CFLAGS) -c download.c

surfer.o: surfer.c callbacks.h surfer.h ../config.h
	$(CC) $(surfer_CFLAGS) -c surfer.c

ghtml.o: ghtml.c ghtml.h ghtml-gtkhtml.c ghtml-gtkmozembed.c ghtml-gtktextview.c ghtml-webkit.c ../config.h
	$(CC) $(surfer_CFLAGS) -c ghtml.c

callbacks.o: callbacks.c surfer.h ghtml.h callbacks.h ../config.h
	$(CC) $(surfer_CFLAGS) -c callbacks.c

main.o: main.c surfer.h ../config.h
	$(CC) $(surfer_CFLAGS) -c main.c

clean:
	$(RM) $(download_OBJS) $(surfer_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 download $(DESTDIR)$(BINDIR)/download
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 surfer $(DESTDIR)$(BINDIR)/surfer

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/download
	$(RM) $(DESTDIR)$(BINDIR)/surfer

.PHONY: all clean distclean install uninstall
