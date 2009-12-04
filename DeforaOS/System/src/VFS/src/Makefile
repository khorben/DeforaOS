TARGETS	= ../data/VFS.h VFS
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I $(PREFIX)/include
CPPFLAGS=
CFLAGSF	= -W -Wall -pedantic
CFLAGS	= -g
LDFLAGSF= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem -Wl,--export-dynamic
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

../data/VFS.h: ../data/VFS.interface
	./appbroker.sh "../data/VFS.h"

VFS_OBJS = vfs.o
VFS_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
VFS_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

VFS: $(VFS_OBJS)
	$(CC) -o VFS $(VFS_OBJS) $(VFS_LDFLAGS)

vfs.o: vfs.c ../data/VFS.h
	$(CC) $(VFS_CFLAGS) -c vfs.c

clean:
	$(RM) $(../data/VFS.h_OBJS) $(VFS_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 VFS $(DESTDIR)$(BINDIR)/VFS

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/VFS

.PHONY: all clean distclean install uninstall
