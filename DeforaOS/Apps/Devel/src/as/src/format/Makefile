TARGETS	= elf.a elf.so flat.a flat.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CFLAGSF	= -W -Wall -fPIC
CFLAGS	= -g
AR	= ar -rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

elf_OBJS = elf.o
elf_CFLAGS = $(CFLAGSF) $(CFLAGS)
elf.a: $(elf_OBJS)
	$(AR) elf.a $(elf_OBJS)
	$(RANLIB) elf.a
elf.so: $(elf_OBJS)
	$(LD) -o elf.so $(elf_OBJS)

flat_OBJS = flat.o
flat_CFLAGS = $(CFLAGSF) $(CFLAGS)
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
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 elf.a $(DESTDIR)$(LIBDIR)/elf.a
	$(INSTALL) -m 0755 elf.so $(DESTDIR)$(LIBDIR)/elf.so
	$(INSTALL) -m 0644 flat.a $(DESTDIR)$(LIBDIR)/flat.a
	$(INSTALL) -m 0755 flat.so $(DESTDIR)$(LIBDIR)/flat.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/elf.a
	$(RM) $(DESTDIR)$(LIBDIR)/elf.so
	$(RM) $(DESTDIR)$(LIBDIR)/flat.a
	$(RM) $(DESTDIR)$(LIBDIR)/flat.so

.PHONY: all clean distclean install uninstall
