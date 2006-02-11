TARGETS	= mktemp w
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

mktemp_OBJS = mktemp.o
mktemp_CFLAGS = $(CFLAGSF) $(CFLAGS)
mktemp: $(mktemp_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o mktemp $(mktemp_OBJS)

w_OBJS = w.o
w_CFLAGS = $(CFLAGSF) $(CFLAGS)
w: $(w_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o w $(w_OBJS)

mktemp.o: mktemp.c
	$(CC) $(mktemp_CFLAGS) -c mktemp.c

w.o: w.c
	$(CC) $(w_CFLAGS) -c w.c

clean:
	$(RM) $(mktemp_OBJS) $(w_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 mktemp $(DESTDIR)$(BINDIR)/mktemp
	$(INSTALL) -m 0755 w $(DESTDIR)$(BINDIR)/w

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/mktemp
	$(RM) $(DESTDIR)$(BINDIR)/w
