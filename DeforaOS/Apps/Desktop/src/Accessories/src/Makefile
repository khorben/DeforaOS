TARGETS	= calendar fontsel run
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

run_OBJS = run.o
run_CFLAGS = $(CFLAGSF) $(CFLAGS)
run: $(run_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o run $(run_OBJS)

calendar.o: calendar.c
	$(CC) $(calendar_CFLAGS) -c calendar.c

fontsel.o: fontsel.c
	$(CC) $(fontsel_CFLAGS) -c fontsel.c

run.o: run.c
	$(CC) $(run_CFLAGS) -c run.c

clean:
	$(RM) $(calendar_OBJS) $(fontsel_OBJS) $(run_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 calendar $(DESTDIR)$(BINDIR)/calendar
	$(INSTALL) -m 0755 fontsel $(DESTDIR)$(BINDIR)/fontsel
	$(INSTALL) -m 0755 run $(DESTDIR)$(BINDIR)/run

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/calendar
	$(RM) $(DESTDIR)$(BINDIR)/fontsel
	$(RM) $(DESTDIR)$(BINDIR)/run

.PHONY: all clean distclean install uninstall
