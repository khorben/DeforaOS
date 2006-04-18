TARGETS	= configure makedepend
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CFLAGSF	= -W -Wall -ansi -D_GNU_SOURCE
CFLAGS	= -g
LDFLAGSF= -l System -l dl
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

configure_OBJS = configure.o makefile.o
configure_CFLAGS = $(CFLAGSF) $(CFLAGS)
configure: $(configure_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o configure $(configure_OBJS)

makedepend_OBJS = makedepend.o
makedepend_CFLAGS = $(CFLAGSF) $(CFLAGS)
makedepend: $(makedepend_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o makedepend $(makedepend_OBJS)

configure.o: configure.c configure.h makefile.h
	$(CC) $(configure_CFLAGS) -c configure.c

makefile.o: makefile.c configure.h
	$(CC) $(configure_CFLAGS) -c makefile.c

makedepend.o: makedepend.c
	$(CC) $(makedepend_CFLAGS) -c makedepend.c

clean:
	$(RM) $(configure_OBJS) $(makedepend_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 configure $(DESTDIR)$(BINDIR)/configure
	$(INSTALL) -m 0755 makedepend $(DESTDIR)$(BINDIR)/makedepend

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/configure
	$(RM) $(DESTDIR)$(BINDIR)/makedepend
