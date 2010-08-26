TARGETS	= keyboard
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs gtk+-2.0` -lXtst
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

keyboard_OBJS = callbacks.o keyboard.o main.o
keyboard_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
keyboard_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

keyboard: $(keyboard_OBJS)
	$(CC) -o keyboard $(keyboard_OBJS) $(keyboard_LDFLAGS)

callbacks.o: callbacks.c
	$(CC) $(keyboard_CFLAGS) -c callbacks.c

keyboard.o: keyboard.c callbacks.h keyboard.h
	$(CC) $(keyboard_CFLAGS) -c keyboard.c

main.o: main.c keyboard.h
	$(CC) $(keyboard_CFLAGS) -c main.c

clean:
	$(RM) -- $(keyboard_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- keyboard $(DESTDIR)$(BINDIR)/keyboard

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/keyboard

.PHONY: all clean distclean install uninstall
