SUBDIRS	= arch format
TARGETS	= libas.a libas.so as disas
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS= 
CFLAGSF	= -W `pkg-config --cflags cpp`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= 
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
BINDIR	= $(PREFIX)/bin
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

libas_OBJS = arch.o as.o code.o format.o parser.o token.o
libas_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -fPIC
libas_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs cpp`

libas.a: $(libas_OBJS)
	$(AR) libas.a $(libas_OBJS)
	$(RANLIB) libas.a

libas.so: $(libas_OBJS)
	$(LD) -o libas.so -Wl,-soname,libas.so.0 $(libas_OBJS) $(libas_LDFLAGS)

as_OBJS = main.o
as_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
as_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L. -Wl,-rpath,$(LIBDIR) -las

as: $(as_OBJS) libas.so
	$(CC) -o as $(as_OBJS) $(as_LDFLAGS)

disas_OBJS = disas.o
disas_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
disas_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L. -Wl,-rpath,$(LIBDIR) -las

disas: $(disas_OBJS) libas.so
	$(CC) -o disas $(disas_OBJS) $(disas_LDFLAGS)

arch.o: arch.c arch.h ../config.h
	$(CC) $(libas_CFLAGS) -c arch.c

as.o: as.c ../include/As.h code.h parser.h as.h ../config.h
	$(CC) $(libas_CFLAGS) -c as.c

code.o: code.c ../include/As.h arch.h code.h
	$(CC) $(libas_CFLAGS) -c code.c

format.o: format.c format.h
	$(CC) $(libas_CFLAGS) -c format.c

parser.o: parser.c parser.h
	$(CC) $(libas_CFLAGS) -c parser.c

token.o: token.c token.h
	$(CC) $(libas_CFLAGS) -c token.c

main.o: main.c ../include/As.h
	$(CC) $(as_CFLAGS) -c main.c

disas.o: disas.c ../include/As.h
	$(CC) $(disas_CFLAGS) -c disas.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(libas_OBJS) $(as_OBJS) $(disas_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(libas_OBJS) $(as_OBJS) $(disas_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 -- libas.a $(DESTDIR)$(LIBDIR)/libas.a
	$(INSTALL) -m 0755 -- libas.so $(DESTDIR)$(LIBDIR)/libas.so.0.0
	$(LN) -s -- libas.so.0.0 $(DESTDIR)$(LIBDIR)/libas.so.0
	$(LN) -s -- libas.so.0.0 $(DESTDIR)$(LIBDIR)/libas.so
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- as $(DESTDIR)$(BINDIR)/as
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- disas $(DESTDIR)$(BINDIR)/disas

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(LIBDIR)/libas.a
	$(RM) -- $(DESTDIR)$(LIBDIR)/libas.so.0.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libas.so.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libas.so
	$(RM) -- $(DESTDIR)$(BINDIR)/as
	$(RM) -- $(DESTDIR)$(BINDIR)/disas

.PHONY: all subdirs clean distclean install uninstall
