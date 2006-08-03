TARGETS	= Editor
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

Editor_OBJS = editor.o main.o
Editor_CFLAGS = $(CFLAGSF) $(CFLAGS)
Editor: $(Editor_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o Editor $(Editor_OBJS)

editor.o: editor.c
	$(CC) $(Editor_CFLAGS) -c editor.c

main.o: main.c
	$(CC) $(Editor_CFLAGS) -c main.c

clean:
	$(RM) $(Editor_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 Editor $(DESTDIR)$(BINDIR)/Editor

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/Editor

.PHONY: all clean distclean install uninstall
