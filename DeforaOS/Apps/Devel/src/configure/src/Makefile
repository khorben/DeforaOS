TARGETS	= configure makedepend makefile.o
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libSystem`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem`
LDFLAGS	= 
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

configure_OBJS = configure.o settings.o
configure_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
configure_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) makefile.o

configure: $(configure_OBJS) makefile.o
	$(CC) -o configure $(configure_OBJS) $(configure_LDFLAGS)

makedepend_OBJS = makedepend.o
makedepend_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
makedepend_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

makedepend: $(makedepend_OBJS)
	$(CC) -o makedepend $(makedepend_OBJS) $(makedepend_LDFLAGS)

makefile.o_OBJS = makefile.o
makefile.o_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o makefile.o

configure.o: configure.c configure.h makefile.h ../config.h
	$(CC) $(configure_CFLAGS) -c configure.c

settings.o: settings.c settings.h
	$(CC) $(configure_CFLAGS) -c settings.c

makedepend.o: makedepend.c
	$(CC) $(makedepend_CFLAGS) -c makedepend.c

makefile.o: makefile.c configure.h settings.h ../config.h
	$(CC) $(makefile.o_CFLAGS) -c makefile.c

clean:
	$(RM) -- $(configure_OBJS) $(makedepend_OBJS) $(makefile.o_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- configure $(DESTDIR)$(BINDIR)/configure

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/configure

.PHONY: all clean distclean install uninstall
