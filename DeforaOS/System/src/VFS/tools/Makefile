TARGETS	= libvfs.a libvfs.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libvfs_OBJS = libvfs.o
libvfs_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -W -Wall -g -O2 -fPIC
libvfs_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem

libvfs.a: $(libvfs_OBJS)
	$(AR) libvfs.a $(libvfs_OBJS)
	$(RANLIB) libvfs.a

libvfs.so: $(libvfs_OBJS)
	$(LD) -o libvfs.so -Wl,-soname,libvfs.so.0 $(libvfs_OBJS) -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem

libvfs.o: libvfs.c
	$(CC) $(libvfs_CFLAGS) -c libvfs.c

clean:
	$(RM) $(libvfs_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
