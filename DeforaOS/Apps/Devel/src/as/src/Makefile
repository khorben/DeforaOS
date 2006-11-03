SUBDIRS	= arch format
TARGETS	= as
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CFLAGSF	= -W -Wall
CFLAGS	= -g
LDFLAGSF= 
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

as_OBJS = as.o code.o parser.o scanner.o token.o arch/arch.o format/format.o
as_CFLAGS = $(CFLAGSF) $(CFLAGS)
as: $(as_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o as $(as_OBJS)

as.o: as.c as.h parser.h
	$(CC) $(as_CFLAGS) -c as.c

code.o: code.c as.h arch/arch.h code.h
	$(CC) $(as_CFLAGS) -c code.c

parser.o: parser.c parser.h scanner.h
	$(CC) $(as_CFLAGS) -c parser.c

scanner.o: scanner.c as.h token.h
	$(CC) $(as_CFLAGS) -c scanner.c

token.o: token.c token.h
	$(CC) $(as_CFLAGS) -c token.c

arch/arch.o: arch/arch.c
	$(CC) $(as_CFLAGS) -c arch/arch.c -o arch/arch.o

format/format.o: format/format.c format/format.h
	$(CC) $(as_CFLAGS) -c format/format.c -o format/format.o

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) $(as_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) $(as_OBJS)
	$(RM) $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 as $(DESTDIR)$(BINDIR)/as

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) $(DESTDIR)$(BINDIR)/as

.PHONY: all subdirs clean distclean install uninstall
