TARGETS	= configure makedepend
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= -lSystem
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

configure_OBJS = configure.o makefile.o settings.o
configure_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
configure_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

configure: $(configure_OBJS)
	$(CC) -o configure $(configure_OBJS) $(configure_LDFLAGS)

makedepend_OBJS = makedepend.o
makedepend_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
makedepend_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

makedepend: $(makedepend_OBJS)
	$(CC) -o makedepend $(makedepend_OBJS) $(makedepend_LDFLAGS)

configure.o: configure.c configure.h makefile.h ../config.h
	$(CC) $(configure_CFLAGS) -c configure.c

makefile.o: makefile.c configure.h settings.h ../config.h
	$(CC) $(configure_CFLAGS) -c makefile.c

settings.o: settings.c settings.h
	$(CC) $(configure_CFLAGS) -c settings.c

makedepend.o: makedepend.c
	$(CC) $(makedepend_CFLAGS) -c makedepend.c

clean:
	$(RM) $(configure_OBJS) $(makedepend_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 configure $(DESTDIR)$(BINDIR)/configure

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/configure

.PHONY: all clean distclean install uninstall
