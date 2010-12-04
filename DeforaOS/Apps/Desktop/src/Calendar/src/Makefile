TARGETS	= calendar
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -W `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config gtk+-2.0 --libs` -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

calendar_OBJS = calendar.o
calendar_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
calendar_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

calendar: $(calendar_OBJS)
	$(CC) -o calendar $(calendar_OBJS) $(calendar_LDFLAGS)

calendar.o: calendar.c
	$(CC) $(calendar_CFLAGS) -c calendar.c

clean:
	$(RM) -- $(calendar_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- calendar $(DESTDIR)$(BINDIR)/calendar

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/calendar

.PHONY: all clean distclean install uninstall
