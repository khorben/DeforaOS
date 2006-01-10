TARGETS	= VFS
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CFLAGSF	= -W -Wall -ansi -I /usr/local/include/libSystem
CFLAGS	= -g
LDFLAGSF= -L /System/Libraries -l System -rdynamic -l dl
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

VFS_OBJS = vfs.o
VFS_CFLAGS = $(CFLAGSF) $(CFLAGS)
VFS: $(VFS_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o VFS $(VFS_OBJS)

vfs.o: vfs.c
	$(CC) $(VFS_CFLAGS) -c vfs.c

clean:
	$(RM) $(VFS_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 VFS $(DESTDIR)$(BINDIR)/VFS

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/VFS
