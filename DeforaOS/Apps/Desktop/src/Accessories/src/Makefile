TARGETS	= calendar fontsel progress run
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -Wall -ansi `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -g
LDFLAGSF= `pkg-config gtk+-2.0 --libs`
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

calendar_OBJS = calendar.o
calendar_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
calendar_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

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

run_OBJS = run.o
run_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
run_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -l System

run: $(run_OBJS)
	$(CC) -o run $(run_OBJS) $(run_LDFLAGS)

calendar.o: calendar.c
	$(CC) $(calendar_CFLAGS) -c calendar.c

fontsel.o: fontsel.c
	$(CC) $(fontsel_CFLAGS) -c fontsel.c

progress.o: progress.c
	$(CC) $(progress_CFLAGS) -c progress.c

run.o: run.c
	$(CC) $(run_CFLAGS) -c run.c

clean:
	$(RM) $(calendar_OBJS) $(fontsel_OBJS) $(progress_OBJS) $(run_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 calendar $(DESTDIR)$(BINDIR)/calendar
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 fontsel $(DESTDIR)$(BINDIR)/fontsel
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 progress $(DESTDIR)$(BINDIR)/progress
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 run $(DESTDIR)$(BINDIR)/run

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/calendar
	$(RM) $(DESTDIR)$(BINDIR)/fontsel
	$(RM) $(DESTDIR)$(BINDIR)/progress
	$(RM) $(DESTDIR)$(BINDIR)/run

.PHONY: all clean distclean install uninstall
