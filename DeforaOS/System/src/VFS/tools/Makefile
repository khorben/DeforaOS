TARGETS	= libVFS.a libVFS.so.0.0 libVFS.so.0 libVFS.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
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

libVFS_OBJS = libvfs.o
libVFS_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libVFS_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

libVFS.a: $(libVFS_OBJS)
	$(AR) -rc libVFS.a $(libVFS_OBJS)
	$(RANLIB) libVFS.a

libVFS.so.0.0 libVFS.so.0 libVFS.so: $(libVFS_OBJS)
	$(CCSHARED) -o libVFS.so.0.0 -Wl,-soname,libVFS.so.0 $(libVFS_OBJS) $(libVFS_LDFLAGS)
	$(LN) -s -- libVFS.so.0.0 libVFS.so.0
	$(LN) -s -- libVFS.so.0.0 libVFS.so

libvfs.o: libvfs.c ../src/common.c ../include/VFS.h
	$(CC) $(libVFS_CFLAGS) -c libvfs.c

clean:
	$(RM) -- $(libVFS_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
