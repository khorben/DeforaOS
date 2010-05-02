TARGETS	= libvfs.a libvfs.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../include -I $(PREFIX)/include
CPPFLAGS=
CFLAGS	= -W -Wall -g -O2 -fPIC
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libvfs_OBJS = libvfs.o
libvfs_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libvfs_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

libvfs.a: $(libvfs_OBJS)
	$(AR) libvfs.a $(libvfs_OBJS)
	$(RANLIB) libvfs.a

libvfs.so: $(libvfs_OBJS)
	$(LD) -o libvfs.so -Wl,-soname,libvfs.so.0 $(libvfs_OBJS)

libvfs.o: libvfs.c ../src/common.c ../include/VFS.h
	$(CC) $(libvfs_CFLAGS) -c libvfs.c

clean:
	$(RM) $(libvfs_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
