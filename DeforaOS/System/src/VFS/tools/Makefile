TARGETS	= libvfs.a libvfs.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -W -fPIC `pkg-config --cflags libApp`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libApp`
LDFLAGS	= 
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

libvfs_OBJS = libvfs.o
libvfs_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libvfs_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

libvfs.a: $(libvfs_OBJS)
	$(AR) -rc libvfs.a $(libvfs_OBJS)
	$(RANLIB) libvfs.a

libvfs.so: $(libvfs_OBJS)
	$(CCSHARED) -o libvfs.so -Wl,-soname,libvfs.so.0 $(libvfs_OBJS) $(libvfs_LDFLAGS)

libvfs.o: libvfs.c ../src/common.c ../include/VFS.h
	$(CC) $(libvfs_CFLAGS) -c libvfs.c

clean:
	$(RM) -- $(libvfs_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
