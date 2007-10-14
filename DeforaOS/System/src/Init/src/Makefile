TARGETS	= Init
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGS=
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -l System -Wl,--export-dynamic 
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

Init_OBJS = init.o
Init_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

Init: $(Init_OBJS)
	$(CC) -o Init $(Init_OBJS) $(LDFLAGSF) $(LDFLAGS)

init.o: init.c
	$(CC) $(Init_CFLAGS) -c init.c

clean:
	$(RM) $(Init_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 Init $(DESTDIR)$(BINDIR)/Init

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/Init

.PHONY: all clean distclean install uninstall
