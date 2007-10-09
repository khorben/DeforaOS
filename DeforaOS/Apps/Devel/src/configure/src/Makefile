TARGETS	= configure
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -l System 
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

configure_OBJS = configure.o makefile.o settings.o
configure_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

configure: $(configure_OBJS)
	$(CC) -o configure $(configure_OBJS) $(LDFLAGSF) $(LDFLAGS)

configure.o: configure.c configure.h makefile.h
	$(CC) $(configure_CFLAGS) -c configure.c

makefile.o: makefile.c configure.h settings.h
	$(CC) $(configure_CFLAGS) -c makefile.c

settings.o: settings.c settings.h
	$(CC) $(configure_CFLAGS) -c settings.c

clean:
	$(RM) $(configure_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 configure $(DESTDIR)$(BINDIR)/configure

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/configure

.PHONY: all clean distclean install uninstall
