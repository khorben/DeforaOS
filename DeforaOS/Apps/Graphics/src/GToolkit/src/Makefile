TARGETS	= libGToolkit.a libGToolkit.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags gl`
CFLAGS	= -Wall -fPIC -g -O2 -pedantic
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libGToolkit_OBJS = common.o gwindow.o gtoolkit.o
libGToolkit_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libGToolkit_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs gl`

libGToolkit.a: $(libGToolkit_OBJS)
	$(AR) libGToolkit.a $(libGToolkit_OBJS)
	$(RANLIB) libGToolkit.a

libGToolkit.so: $(libGToolkit_OBJS)
	$(LD) -o libGToolkit.so -Wl,-soname,libGToolkit.so.0 $(libGToolkit_OBJS) `pkg-config --libs gl`

common.o: common.c
	$(CC) $(libGToolkit_CFLAGS) -c common.c

gwindow.o: gwindow.c
	$(CC) $(libGToolkit_CFLAGS) -c gwindow.c

gtoolkit.o: gtoolkit.c
	$(CC) $(libGToolkit_CFLAGS) -c gtoolkit.c

clean:
	$(RM) -- $(libGToolkit_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 0644 -- libGToolkit.a $(DESTDIR)$(PREFIX)/lib/libGToolkit.a
	$(INSTALL) -m 0755 -- libGToolkit.so $(DESTDIR)$(PREFIX)/lib/libGToolkit.so.0.0
	$(LN) -s -- libGToolkit.so.0.0 $(DESTDIR)$(PREFIX)/lib/libGToolkit.so.0
	$(LN) -s -- libGToolkit.so.0.0 $(DESTDIR)$(PREFIX)/lib/libGToolkit.so

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libGToolkit.a
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libGToolkit.so.0.0
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libGToolkit.so.0
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libGToolkit.so

.PHONY: all clean distclean install uninstall
