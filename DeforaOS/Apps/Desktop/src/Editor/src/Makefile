TARGETS	= editor
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config --libs gtk+-2.0`
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

editor_OBJS = editor.o main.o
editor_CFLAGS = $(CFLAGSF) $(CFLAGS)
editor: $(editor_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o editor $(editor_OBJS)

editor.o: editor.c ../config.h
	$(CC) $(editor_CFLAGS) -c editor.c

main.o: main.c
	$(CC) $(editor_CFLAGS) -c main.c

clean:
	$(RM) $(editor_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 editor $(DESTDIR)$(BINDIR)/editor

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/editor

.PHONY: all clean distclean install uninstall
