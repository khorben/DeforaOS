TARGETS	= editor
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I $(PREFIX)/include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs gtk+-2.0` -lDesktop
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

editor_OBJS = callbacks.o editor.o main.o
editor_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
editor_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

editor: $(editor_OBJS)
	$(CC) -o editor $(editor_OBJS) $(editor_LDFLAGS)

callbacks.o: callbacks.c callbacks.h
	$(CC) $(editor_CFLAGS) -c callbacks.c

editor.o: editor.c callbacks.h editor.h ../config.h
	$(CC) $(editor_CFLAGS) -c editor.c

main.o: main.c
	$(CC) $(editor_CFLAGS) -c main.c

clean:
	$(RM) -- $(editor_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- editor $(DESTDIR)$(BINDIR)/editor

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/editor

.PHONY: all clean distclean install uninstall
