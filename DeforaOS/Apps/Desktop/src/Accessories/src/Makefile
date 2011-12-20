TARGETS	= compare fontsel progress
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config gtk+-3.0 --cflags`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config gtk+-3.0 --libs`
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

compare_OBJS = compare.o
compare_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
compare_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

compare: $(compare_OBJS)
	$(CC) -o compare $(compare_OBJS) $(compare_LDFLAGS)

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

compare.o: compare.c
	$(CC) $(compare_CFLAGS) -c compare.c

fontsel.o: fontsel.c
	$(CC) $(fontsel_CFLAGS) -c fontsel.c

progress.o: progress.c
	$(CC) $(progress_CFLAGS) -c progress.c

clean:
	$(RM) -- $(compare_OBJS) $(fontsel_OBJS) $(progress_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- compare $(DESTDIR)$(BINDIR)/compare
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- fontsel $(DESTDIR)$(BINDIR)/fontsel
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- progress $(DESTDIR)$(BINDIR)/progress

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/compare
	$(RM) -- $(DESTDIR)$(BINDIR)/fontsel
	$(RM) -- $(DESTDIR)$(BINDIR)/progress

.PHONY: all clean distclean install uninstall
