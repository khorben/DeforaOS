TARGETS	= editor
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libSystem libDesktop`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem libDesktop`
LDFLAGS	= 
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

editor_OBJS = callbacks.o editor.o main.o
editor_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
editor_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

editor: $(editor_OBJS)
	$(CC) -o editor $(editor_OBJS) $(editor_LDFLAGS)

callbacks.o: callbacks.c editor.h callbacks.h
	$(CC) $(editor_CFLAGS) -c callbacks.c

editor.o: editor.c callbacks.h editor.h ../config.h
	$(CC) $(editor_CFLAGS) -c editor.c

main.o: main.c
	$(CC) $(editor_CFLAGS) -c main.c

clean:
	$(RM) -- $(editor_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- editor $(DESTDIR)$(BINDIR)/editor

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/editor

.PHONY: all clean distclean install uninstall
