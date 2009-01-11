TARGETS	= VFS
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -l System -Wl,--export-dynamic 
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

VFS_OBJS = vfs.o
VFS_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
VFS_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

VFS: $(VFS_OBJS)
	$(CC) -o VFS $(VFS_OBJS) $(VFS_LDFLAGS)

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

.PHONY: all clean distclean install uninstall
