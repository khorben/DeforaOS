TARGETS	= libGToolkit.a libGToolkit.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -I ../include
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
libGToolkit_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

libGToolkit.a: $(libGToolkit_OBJS)
	$(AR) libGToolkit.a $(libGToolkit_OBJS)
	$(RANLIB) libGToolkit.a

libGToolkit.so: $(libGToolkit_OBJS)
	$(LD) -o libGToolkit.so -Wl,-soname,libGToolkit.so.0 $(libGToolkit_OBJS)

common.o: common.c
	$(CC) $(libGToolkit_CFLAGS) -c common.c

gwindow.o: gwindow.c
	$(CC) $(libGToolkit_CFLAGS) -c gwindow.c

gtoolkit.o: gtoolkit.c
	$(CC) $(libGToolkit_CFLAGS) -c gtoolkit.c

clean:
	$(RM) $(libGToolkit_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
