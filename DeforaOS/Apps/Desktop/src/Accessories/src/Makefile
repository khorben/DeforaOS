TARGETS	= calendar fontsel
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CFLAGSF	= -W -Wall -ansi `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -g
LDFLAGSF= `pkg-config gtk+-2.0 --libs`
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

calendar_OBJS = calendar.o
calendar_CFLAGS = $(CFLAGSF) $(CFLAGS)
calendar: $(calendar_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o calendar $(calendar_OBJS)

fontsel_OBJS = fontsel.o
fontsel_CFLAGS = $(CFLAGSF) $(CFLAGS)
fontsel: $(fontsel_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o fontsel $(fontsel_OBJS)

calendar.o: calendar.c
	$(CC) $(calendar_CFLAGS) -c calendar.c

fontsel.o: fontsel.c
	$(CC) $(fontsel_CFLAGS) -c fontsel.c

clean:
	$(RM) $(calendar_OBJS) $(fontsel_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 calendar $(DESTDIR)$(BINDIR)/calendar
	$(INSTALL) -m 0755 fontsel $(DESTDIR)$(BINDIR)/fontsel

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/calendar
	$(RM) $(DESTDIR)$(BINDIR)/fontsel
