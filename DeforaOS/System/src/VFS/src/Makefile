TARGETS	= ../data/VFS.h VFS
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include -I $(PREFIX)/include
CPPFLAGS=
CFLAGSF	= -W -Wall -pedantic
CFLAGS	= -g
LDFLAGSF= -lSystem -Wl,--export-dynamic
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

../data/VFS.h: ../data/VFS.interface
	./appbroker.sh "../data/VFS.h"

VFS_OBJS = vfs.o main.o
VFS_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
VFS_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

VFS: $(VFS_OBJS)
	$(CC) -o VFS $(VFS_OBJS) $(VFS_LDFLAGS)

vfs.o: vfs.c common.c ../data/VFS.h ../include/VFS.h
	$(CC) $(VFS_CFLAGS) -c vfs.c

main.o: main.c
	$(CC) $(VFS_CFLAGS) -c main.c

clean:
	$(RM) -- $(../data/VFS.h_OBJS) $(VFS_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- VFS $(DESTDIR)$(BINDIR)/VFS

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/VFS

.PHONY: all clean distclean install uninstall
