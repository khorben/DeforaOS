TARGETS	= openmoko.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../../include
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -fPIC
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

openmoko_OBJS = openmoko.o
openmoko_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
openmoko_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

openmoko.so: $(openmoko_OBJS)
	$(LD) -o openmoko.so $(openmoko_OBJS)

openmoko.o: openmoko.c ../../include/Phone.h
	$(CC) $(openmoko_CFLAGS) -c openmoko.c

clean:
	$(RM) $(openmoko_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 openmoko.so $(DESTDIR)$(LIBDIR)/Phone/plugins/openmoko.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/Phone/plugins/openmoko.so

.PHONY: all clean distclean install uninstall
