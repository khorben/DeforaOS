TARGETS	= calendar fontsel progress
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -Wall -pedantic `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -g
LDFLAGSF= `pkg-config gtk+-2.0 --libs`
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

calendar_OBJS = calendar.o
calendar_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -I $(PREFIX)/include $(CFLAGSF) $(CFLAGS)
calendar_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem

calendar: $(calendar_OBJS)
	$(CC) -o calendar $(calendar_OBJS) $(calendar_LDFLAGS)

fontsel_OBJS = fontsel.o
fontsel_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
fontsel_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

fontsel: $(fontsel_OBJS)
	$(CC) -o fontsel $(fontsel_OBJS) $(fontsel_LDFLAGS)

progress_OBJS = progress.o
progress_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
progress_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

progress: $(progress_OBJS)
	$(CC) -o progress $(progress_OBJS) $(progress_LDFLAGS)

calendar.o: calendar.c
	$(CC) $(calendar_CFLAGS) -c calendar.c

fontsel.o: fontsel.c
	$(CC) $(fontsel_CFLAGS) -c fontsel.c

progress.o: progress.c
	$(CC) $(progress_CFLAGS) -c progress.c

clean:
	$(RM) -- $(calendar_OBJS) $(fontsel_OBJS) $(progress_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- calendar $(DESTDIR)$(BINDIR)/calendar
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- fontsel $(DESTDIR)$(BINDIR)/fontsel
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- progress $(DESTDIR)$(BINDIR)/progress

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/calendar
	$(RM) -- $(DESTDIR)$(BINDIR)/fontsel
	$(RM) -- $(DESTDIR)$(BINDIR)/progress

.PHONY: all clean distclean install uninstall
