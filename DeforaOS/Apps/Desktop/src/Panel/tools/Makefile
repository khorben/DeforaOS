TARGETS	= panel_test
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config --libs libSystem gtk+-2.0`
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

panel_test_OBJS = test.o
panel_test_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D PREFIX=\"$(PREFIX)\" $(CFLAGSF) $(CFLAGS)
panel_test_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

panel_test: $(panel_test_OBJS)
	$(CC) -o panel_test $(panel_test_OBJS) $(panel_test_LDFLAGS)

test.o: test.c ../src/common.h ../config.h
	$(CC) $(panel_test_CFLAGS) -c test.c

clean:
	$(RM) -- $(panel_test_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- panel_test $(DESTDIR)$(BINDIR)/panel_test

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/panel_test

.PHONY: all clean distclean install uninstall
