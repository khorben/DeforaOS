TARGETS	= amd64.a amd64.so i386.a i386.so i386_real.a i386_real.so i486.a i486.so i586.a i586.so i686.a i686.so sparc.a sparc.so sparc64.a sparc64.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -Wall -ansi -fPIC
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

amd64_OBJS = amd64.o
amd64_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

amd64.a: $(amd64_OBJS)
	$(AR) amd64.a $(amd64_OBJS)
	$(RANLIB) amd64.a

amd64.so: $(amd64_OBJS)
	$(LD) -o amd64.so $(amd64_OBJS)

i386_OBJS = i386.o
i386_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

i386.a: $(i386_OBJS)
	$(AR) i386.a $(i386_OBJS)
	$(RANLIB) i386.a

i386.so: $(i386_OBJS)
	$(LD) -o i386.so $(i386_OBJS)

i386_real_OBJS = i386_real.o
i386_real_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

i386_real.a: $(i386_real_OBJS)
	$(AR) i386_real.a $(i386_real_OBJS)
	$(RANLIB) i386_real.a

i386_real.so: $(i386_real_OBJS)
	$(LD) -o i386_real.so $(i386_real_OBJS)

i486_OBJS = i486.o
i486_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

i486.a: $(i486_OBJS)
	$(AR) i486.a $(i486_OBJS)
	$(RANLIB) i486.a

i486.so: $(i486_OBJS)
	$(LD) -o i486.so $(i486_OBJS)

i586_OBJS = i586.o
i586_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

i586.a: $(i586_OBJS)
	$(AR) i586.a $(i586_OBJS)
	$(RANLIB) i586.a

i586.so: $(i586_OBJS)
	$(LD) -o i586.so $(i586_OBJS)

i686_OBJS = i686.o
i686_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

i686.a: $(i686_OBJS)
	$(AR) i686.a $(i686_OBJS)
	$(RANLIB) i686.a

i686.so: $(i686_OBJS)
	$(LD) -o i686.so $(i686_OBJS)

sparc_OBJS = sparc.o
sparc_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

sparc.a: $(sparc_OBJS)
	$(AR) sparc.a $(sparc_OBJS)
	$(RANLIB) sparc.a

sparc.so: $(sparc_OBJS)
	$(LD) -o sparc.so $(sparc_OBJS)

sparc64_OBJS = sparc64.o
sparc64_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

sparc64.a: $(sparc64_OBJS)
	$(AR) sparc64.a $(sparc64_OBJS)
	$(RANLIB) sparc64.a

sparc64.so: $(sparc64_OBJS)
	$(LD) -o sparc64.so $(sparc64_OBJS)

amd64.o: amd64.c arch.h 80386.ins 80386.reg 80486.ins 80486.reg 80586.ins 80586.reg 80686.ins 80686.reg
	$(CC) $(amd64_CFLAGS) -c amd64.c

i386.o: i386.c arch.h 80386.ins 80386.reg
	$(CC) $(i386_CFLAGS) -c i386.c

i386_real.o: i386_real.c arch.h 80386.ins 80386.reg
	$(CC) $(i386_real_CFLAGS) -c i386_real.c

i486.o: i486.c arch.h 80386.ins 80386.reg 80486.ins 80486.reg
	$(CC) $(i486_CFLAGS) -c i486.c

i586.o: i586.c arch.h 80386.ins 80386.reg 80486.ins 80486.reg 80586.ins 80586.reg
	$(CC) $(i586_CFLAGS) -c i586.c

i686.o: i686.c arch.h 80386.ins 80386.reg 80486.ins 80486.reg 80586.ins 80586.reg 80686.ins 80686.reg
	$(CC) $(i686_CFLAGS) -c i686.c

sparc.o: sparc.c arch.h sparc.ins sparc.reg
	$(CC) $(sparc_CFLAGS) -c sparc.c

sparc64.o: sparc64.c arch.h sparc.ins sparc.reg sparc64.ins sparc64.reg
	$(CC) $(sparc64_CFLAGS) -c sparc64.c

clean:
	$(RM) $(amd64_OBJS) $(i386_OBJS) $(i386_real_OBJS) $(i486_OBJS) $(i586_OBJS) $(i686_OBJS) $(sparc_OBJS) $(sparc64_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/arch
	$(INSTALL) -m 0644 amd64.a $(DESTDIR)$(LIBDIR)/as/arch/amd64.a
	$(INSTALL) -m 0755 amd64.so $(DESTDIR)$(LIBDIR)/as/arch/amd64.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/arch
	$(INSTALL) -m 0644 i386.a $(DESTDIR)$(LIBDIR)/as/arch/i386.a
	$(INSTALL) -m 0755 i386.so $(DESTDIR)$(LIBDIR)/as/arch/i386.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/arch
	$(INSTALL) -m 0644 i386_real.a $(DESTDIR)$(LIBDIR)/as/arch/i386_real.a
	$(INSTALL) -m 0755 i386_real.so $(DESTDIR)$(LIBDIR)/as/arch/i386_real.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/arch
	$(INSTALL) -m 0644 i486.a $(DESTDIR)$(LIBDIR)/as/arch/i486.a
	$(INSTALL) -m 0755 i486.so $(DESTDIR)$(LIBDIR)/as/arch/i486.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/arch
	$(INSTALL) -m 0644 i586.a $(DESTDIR)$(LIBDIR)/as/arch/i586.a
	$(INSTALL) -m 0755 i586.so $(DESTDIR)$(LIBDIR)/as/arch/i586.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/arch
	$(INSTALL) -m 0644 i686.a $(DESTDIR)$(LIBDIR)/as/arch/i686.a
	$(INSTALL) -m 0755 i686.so $(DESTDIR)$(LIBDIR)/as/arch/i686.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/arch
	$(INSTALL) -m 0644 sparc.a $(DESTDIR)$(LIBDIR)/as/arch/sparc.a
	$(INSTALL) -m 0755 sparc.so $(DESTDIR)$(LIBDIR)/as/arch/sparc.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/arch
	$(INSTALL) -m 0644 sparc64.a $(DESTDIR)$(LIBDIR)/as/arch/sparc64.a
	$(INSTALL) -m 0755 sparc64.so $(DESTDIR)$(LIBDIR)/as/arch/sparc64.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/amd64.a
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/amd64.so
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/i386.a
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/i386.so
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/i386_real.a
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/i386_real.so
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/i486.a
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/i486.so
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/i586.a
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/i586.so
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/i686.a
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/i686.so
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/sparc.a
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/sparc.so
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/sparc64.a
	$(RM) $(DESTDIR)$(LIBDIR)/as/arch/sparc64.so

.PHONY: all clean distclean install uninstall
