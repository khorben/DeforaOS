TARGETS	= i386.a i386.so i486.a i486.so sparc.a sparc.so sparc64.a sparc64.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	= cc
CFLAGSF	= -W -Wall
AR	= ar -rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

i386_OBJS = i386.o
i386_CFLAGS = $(CFLAGSF) $(CFLAGS)
i386.a: $(i386_OBJS)
	$(AR) i386.a $(i386_OBJS)
	$(RANLIB) i386.a
i386.so: $(i386_OBJS)
	$(LD) -o i386.so $(i386_OBJS)

i486_OBJS = i486.o
i486_CFLAGS = $(CFLAGSF) $(CFLAGS)
i486.a: $(i486_OBJS)
	$(AR) i486.a $(i486_OBJS)
	$(RANLIB) i486.a
i486.so: $(i486_OBJS)
	$(LD) -o i486.so $(i486_OBJS)

sparc_OBJS = sparc.o
sparc_CFLAGS = $(CFLAGSF) $(CFLAGS)
sparc.a: $(sparc_OBJS)
	$(AR) sparc.a $(sparc_OBJS)
	$(RANLIB) sparc.a
sparc.so: $(sparc_OBJS)
	$(LD) -o sparc.so $(sparc_OBJS)

sparc64_OBJS = sparc64.o
sparc64_CFLAGS = $(CFLAGSF) $(CFLAGS)
sparc64.a: $(sparc64_OBJS)
	$(AR) sparc64.a $(sparc64_OBJS)
	$(RANLIB) sparc64.a
sparc64.so: $(sparc64_OBJS)
	$(LD) -o sparc64.so $(sparc64_OBJS)

i386.o: i386.c
	$(CC) $(i386_CFLAGS) -c i386.c

i486.o: i486.c
	$(CC) $(i486_CFLAGS) -c i486.c

sparc.o: sparc.c
	$(CC) $(sparc_CFLAGS) -c sparc.c

sparc64.o: sparc64.c
	$(CC) $(sparc64_CFLAGS) -c sparc64.c

clean:
	$(RM) $(i386_OBJS) $(i486_OBJS) $(sparc_OBJS) $(sparc64_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 i386.a $(DESTDIR)$(LIBDIR)/i386.a
	$(INSTALL) -m 0755 i386.so $(DESTDIR)$(LIBDIR)/i386.so
	$(INSTALL) -m 0644 i486.a $(DESTDIR)$(LIBDIR)/i486.a
	$(INSTALL) -m 0755 i486.so $(DESTDIR)$(LIBDIR)/i486.so
	$(INSTALL) -m 0644 sparc.a $(DESTDIR)$(LIBDIR)/sparc.a
	$(INSTALL) -m 0755 sparc.so $(DESTDIR)$(LIBDIR)/sparc.so
	$(INSTALL) -m 0644 sparc64.a $(DESTDIR)$(LIBDIR)/sparc64.a
	$(INSTALL) -m 0755 sparc64.so $(DESTDIR)$(LIBDIR)/sparc64.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/i386.a
	$(RM) $(DESTDIR)$(LIBDIR)/i386.so
	$(RM) $(DESTDIR)$(LIBDIR)/i486.a
	$(RM) $(DESTDIR)$(LIBDIR)/i486.so
	$(RM) $(DESTDIR)$(LIBDIR)/sparc.a
	$(RM) $(DESTDIR)$(LIBDIR)/sparc.so
	$(RM) $(DESTDIR)$(LIBDIR)/sparc64.a
	$(RM) $(DESTDIR)$(LIBDIR)/sparc64.so
