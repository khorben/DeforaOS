TARGETS	= libParser.a libParser.so.0.0 libParser.so.0 libParser.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../include -I $(PREFIX)/include
CPPFLAGS?=
CFLAGSF	= -W -fPIC
CFLAGS	= -Wall -g -O2 -pedantic
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

libParser_OBJS = xml.o
libParser_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libParser_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem

libParser.a: $(libParser_OBJS)
	$(AR) -rc libParser.a $(libParser_OBJS)
	$(RANLIB) libParser.a

libParser.so.0.0 libParser.so.0 libParser.so: $(libParser_OBJS)
	$(CCSHARED) -o libParser.so.0.0 -Wl,-soname,libParser.so.0 $(libParser_OBJS) $(libParser_LDFLAGS)
	$(LN) -s -- libParser.so.0.0 libParser.so.0
	$(LN) -s -- libParser.so.0.0 libParser.so

xml.o: xml.c ../include/System/Parser/XML.h
	$(CC) $(libParser_CFLAGS) -c xml.c

clean:
	$(RM) -- $(libParser_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 0644 -- libParser.a $(DESTDIR)$(PREFIX)/lib/libParser.a
	$(INSTALL) -m 0755 -- libParser.so.0.0 $(DESTDIR)$(PREFIX)/lib/libParser.so.0.0
	$(LN) -s -- libParser.so.0.0 $(DESTDIR)$(PREFIX)/lib/libParser.so.0
	$(LN) -s -- libParser.so.0.0 $(DESTDIR)$(PREFIX)/lib/libParser.so

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libParser.a
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libParser.so.0.0
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libParser.so.0
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libParser.so

.PHONY: all clean distclean install uninstall
