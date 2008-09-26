SUBDIRS	= arch format
TARGETS	= libas.a libas.so as
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF=
CPPFLAGS= -I ../include
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
BINDIR	= $(PREFIX)/bin
LDFLAGSF= 
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

libas_OBJS = as.o code.o parser.o token.o arch/arch.o format/format.o
libas_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -fPIC

libas.a: $(libas_OBJS)
	$(AR) libas.a $(libas_OBJS)
	$(RANLIB) libas.a

libas.so: $(libas_OBJS)
	$(LD) -o libas.so $(libas_OBJS) -L $(LIBDIR) -Wl,-rpath $(LIBDIR) -l cpp

as_OBJS = main.o
as_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

as: $(as_OBJS) libas.so
	$(CC) -o as $(as_OBJS) $(LDFLAGSF) $(LDFLAGS) -L . -Wl,-rpath,$(LIBDIR) -l as

as.o: as.c ../include/as.h ../config.h
	$(CC) $(libas_CFLAGS) -c as.c

code.o: code.c ../include/as.h arch/arch.h code.h
	$(CC) $(libas_CFLAGS) -c code.c

parser.o: parser.c parser.h
	$(CC) $(libas_CFLAGS) -c parser.c

token.o: token.c token.h
	$(CC) $(libas_CFLAGS) -c token.c

arch/arch.o: arch/arch.c
	$(CC) $(libas_CFLAGS) -o arch/arch.o -c arch/arch.c

format/format.o: format/format.c format/format.h
	$(CC) $(libas_CFLAGS) -o format/format.o -c format/format.c

main.o: main.c ../include/as.h
	$(CC) $(as_CFLAGS) -c main.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) $(libas_OBJS) $(as_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) $(libas_OBJS) $(as_OBJS)
	$(RM) $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 libas.a $(DESTDIR)$(LIBDIR)/libas.a
	$(INSTALL) -m 0755 libas.so $(DESTDIR)$(LIBDIR)/libas.so
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 as $(DESTDIR)$(BINDIR)/as

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) $(DESTDIR)$(LIBDIR)/libas.a
	$(RM) $(DESTDIR)$(LIBDIR)/libas.so
	$(RM) $(DESTDIR)$(BINDIR)/as

.PHONY: all subdirs clean distclean install uninstall
