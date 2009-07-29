SUBDIRS	= sets target
TARGETS	= c99
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS= -I ../include
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= -L $(PREFIX)/lib -Wl,-rpath $(PREFIX)/lib -l cpp
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

c99_OBJS = c99.o code.o main.o parser.o scanner.o tokenset.o
c99_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
c99_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

c99: $(c99_OBJS)
	$(CC) -o c99 $(c99_OBJS) $(c99_LDFLAGS)

c99.o: c99.c common.h code.h parser.h ../include/c99.h
	$(CC) $(c99_CFLAGS) -c c99.c

code.o: code.c ../include/c99/target.h code.h ../config.h
	$(CC) $(c99_CFLAGS) -c code.c

main.o: main.c ../include/c99.h ../config.h
	$(CC) $(c99_CFLAGS) -c main.c

parser.o: parser.c common.h code.h tokenset.h scanner.h ../include/c99.h ../config.h
	$(CC) $(c99_CFLAGS) -c parser.c

scanner.o: scanner.c ../include/c99.h ../config.h
	$(CC) $(c99_CFLAGS) -c scanner.c

tokenset.o: tokenset.c common.h code.h tokenset.h
	$(CC) $(c99_CFLAGS) -c tokenset.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) $(c99_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) $(c99_OBJS)
	$(RM) $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 c99 $(DESTDIR)$(BINDIR)/c99

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) $(DESTDIR)$(BINDIR)/c99

.PHONY: all subdirs clean distclean install uninstall
