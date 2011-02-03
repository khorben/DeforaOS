TARGETS	= elf.so flat.so java.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I $(PREFIX)/include
CPPFLAGS=
CFLAGSF	= -W -Wall -fPIC
CFLAGS	= -g
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

elf_OBJS = elf.o
elf_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
elf_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

elf.so: $(elf_OBJS)
	$(LD) -o elf.so $(elf_OBJS) $(elf_LDFLAGS)

flat_OBJS = flat.o
flat_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
flat_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

flat.so: $(flat_OBJS)
	$(LD) -o flat.so $(flat_OBJS) $(flat_LDFLAGS)

java_OBJS = java.o
java_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
java_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

java.so: $(java_OBJS)
	$(LD) -o java.so $(java_OBJS) $(java_LDFLAGS)

elf.o: elf.c format.h
	$(CC) $(elf_CFLAGS) -c elf.c

flat.o: flat.c format.h
	$(CC) $(flat_CFLAGS) -c flat.c

java.o: java.c format.h
	$(CC) $(java_CFLAGS) -c java.c

clean:
	$(RM) -- $(elf_OBJS) $(flat_OBJS) $(java_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/format
	$(INSTALL) -m 0644 -- elf.so $(DESTDIR)$(LIBDIR)/as/format/elf.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/format
	$(INSTALL) -m 0644 -- flat.so $(DESTDIR)$(LIBDIR)/as/format/flat.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/as/format
	$(INSTALL) -m 0644 -- java.so $(DESTDIR)$(LIBDIR)/as/format/java.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/as/format/elf.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/as/format/flat.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/as/format/java.so

.PHONY: all clean distclean install uninstall
