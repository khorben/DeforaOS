TARGETS	= hexdump mktemp tar w
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

hexdump_OBJS = hexdump.o
hexdump_CFLAGS = $(CFLAGSF) $(CFLAGS)
hexdump: $(hexdump_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o hexdump $(hexdump_OBJS)

mktemp_OBJS = mktemp.o
mktemp_CFLAGS = $(CFLAGSF) $(CFLAGS)
mktemp: $(mktemp_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o mktemp $(mktemp_OBJS)

tar_OBJS = tar.o
tar_CFLAGS = $(CFLAGSF) $(CFLAGS)
tar: $(tar_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o tar $(tar_OBJS)

w_OBJS = w.o
w_CFLAGS = $(CFLAGSF) $(CFLAGS)
w: $(w_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o w $(w_OBJS)

hexdump.o: hexdump.c
	$(CC) $(hexdump_CFLAGS) -c hexdump.c

mktemp.o: mktemp.c
	$(CC) $(mktemp_CFLAGS) -c mktemp.c

tar.o: tar.c
	$(CC) $(tar_CFLAGS) -c tar.c

w.o: w.c
	$(CC) $(w_CFLAGS) -c w.c

clean:
	$(RM) $(hexdump_OBJS) $(mktemp_OBJS) $(tar_OBJS) $(w_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 hexdump $(DESTDIR)$(BINDIR)/hexdump
	$(INSTALL) -m 0755 mktemp $(DESTDIR)$(BINDIR)/mktemp
	$(INSTALL) -m 0755 tar $(DESTDIR)$(BINDIR)/tar
	$(INSTALL) -m 0755 w $(DESTDIR)$(BINDIR)/w

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/hexdump
	$(RM) $(DESTDIR)$(BINDIR)/mktemp
	$(RM) $(DESTDIR)$(BINDIR)/tar
	$(RM) $(DESTDIR)$(BINDIR)/w
