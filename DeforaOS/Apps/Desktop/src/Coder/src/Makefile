TARGETS	= coder
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libSystem libDesktop`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem libDesktop`
LDFLAGS	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

coder_OBJS = callbacks.o gedi.o main.o project.o
coder_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
coder_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

coder: $(coder_OBJS)
	$(CC) -o coder $(coder_OBJS) $(coder_LDFLAGS)

callbacks.o: callbacks.c callbacks.h gedi.h project.h
	$(CC) $(coder_CFLAGS) -c callbacks.c

gedi.o: gedi.c callbacks.h gedi.h project.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(coder_CFLAGS) -c gedi.c

main.o: main.c gedi.h
	$(CC) $(coder_CFLAGS) -c main.c

project.o: project.c project.h
	$(CC) $(coder_CFLAGS) -c project.c

clean:
	$(RM) -- $(coder_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- coder $(DESTDIR)$(BINDIR)/coder

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/coder

.PHONY: all clean distclean install uninstall
