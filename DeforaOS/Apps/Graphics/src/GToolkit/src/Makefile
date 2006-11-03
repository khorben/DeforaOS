TARGETS	= libGToolkit.a libGToolkit.so test
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CFLAGSF	= -W -I ../include
CFLAGS	= -Wall -fPIC -g -O2 -ansi
AR	= ar -rc
RANLIB	= ranlib
LD	= ld -shared
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libGToolkit_OBJS = common.o gwindow.o gtoolkit.o
libGToolkit_CFLAGS = $(CFLAGSF) $(CFLAGS)
libGToolkit.a: $(libGToolkit_OBJS)
	$(AR) libGToolkit.a $(libGToolkit_OBJS)
	$(RANLIB) libGToolkit.a
libGToolkit.so: $(libGToolkit_OBJS)
	$(LD) -o libGToolkit.so $(libGToolkit_OBJS)

test_OBJS = test.o
test_CFLAGS = $(CFLAGSF) $(CFLAGS)
test: $(test_OBJS)
	$(CC) $(LDFLAGSF) -L . -l GToolkit -l X11 -l GL $(LDFLAGS) -o test $(test_OBJS)

common.o: common.c
	$(CC) $(libGToolkit_CFLAGS) -c common.c

gwindow.o: gwindow.c
	$(CC) $(libGToolkit_CFLAGS) -c gwindow.c

gtoolkit.o: gtoolkit.c
	$(CC) $(libGToolkit_CFLAGS) -c gtoolkit.c

test.o: test.c ../include/GToolkit.h
	$(CC) $(test_CFLAGS) -c test.c

clean:
	$(RM) $(libGToolkit_OBJS) $(test_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 libGToolkit.a $(DESTDIR)$(LIBDIR)/libGToolkit.a
	$(INSTALL) -m 0755 libGToolkit.so $(DESTDIR)$(LIBDIR)/libGToolkit.so
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 test $(DESTDIR)$(BINDIR)/test

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/libGToolkit.a
	$(RM) $(DESTDIR)$(LIBDIR)/libGToolkit.so
	$(RM) $(DESTDIR)$(BINDIR)/test

.PHONY: all clean distclean install uninstall
