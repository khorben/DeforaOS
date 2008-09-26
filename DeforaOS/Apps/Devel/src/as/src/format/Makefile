TARGETS	= elf.a elf.so flat.a flat.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -Wall -fPIC
CFLAGS	= -g
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

elf_OBJS = elf.o
elf_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

elf.a: $(elf_OBJS)
	$(AR) elf.a $(elf_OBJS)
	$(RANLIB) elf.a

elf.so: $(elf_OBJS)
	$(LD) -o elf.so $(elf_OBJS)

flat_OBJS = flat.o
flat_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

flat.a: $(flat_OBJS)
	$(AR) flat.a $(flat_OBJS)
	$(RANLIB) flat.a

flat.so: $(flat_OBJS)
	$(LD) -o flat.so $(flat_OBJS)

elf.o: elf.c format.h
	$(CC) $(elf_CFLAGS) -c elf.c

flat.o: flat.c format.h
	$(CC) $(flat_CFLAGS) -c flat.c

clean:
	$(RM) $(elf_OBJS) $(flat_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/format
	$(INSTALL) -m 0644 elf.a $(DESTDIR)$(LIBDIR)/as/format/elf.a
	$(INSTALL) -m 0755 elf.so $(DESTDIR)$(LIBDIR)/as/format/elf.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/format
	$(INSTALL) -m 0644 flat.a $(DESTDIR)$(LIBDIR)/as/format/flat.a
	$(INSTALL) -m 0755 flat.so $(DESTDIR)$(LIBDIR)/as/format/flat.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/as/format/elf.a
	$(RM) $(DESTDIR)$(LIBDIR)/as/format/elf.so
	$(RM) $(DESTDIR)$(LIBDIR)/as/format/flat.a
	$(RM) $(DESTDIR)$(LIBDIR)/as/format/flat.so

.PHONY: all clean distclean install uninstall
