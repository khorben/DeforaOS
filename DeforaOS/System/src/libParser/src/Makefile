TARGETS	= libParser.a libParser.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libParser_OBJS = xml.o
libParser_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libParser_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem

libParser.a: $(libParser_OBJS)
	$(AR) libParser.a $(libParser_OBJS)
	$(RANLIB) libParser.a

libParser.so: $(libParser_OBJS)
	$(LD) -o libParser.so -Wl,-soname,libParser.so.0 $(libParser_OBJS) -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem

xml.o: xml.c ../include/System/Parser/XML.h
	$(CC) $(libParser_CFLAGS) -c xml.c

clean:
	$(RM) -- $(libParser_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 0644 -- libParser.a $(DESTDIR)$(PREFIX)/lib/libParser.a
	$(INSTALL) -m 0755 -- libParser.so $(DESTDIR)$(PREFIX)/lib/libParser.so.0.0
	$(LN) -s -- libParser.so.0.0 $(DESTDIR)$(PREFIX)/lib/libParser.so.0
	$(LN) -s -- libParser.so.0.0 $(DESTDIR)$(PREFIX)/lib/libParser.so

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libParser.a
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libParser.so.0.0
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libParser.so.0
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libParser.so

.PHONY: all clean distclean install uninstall
