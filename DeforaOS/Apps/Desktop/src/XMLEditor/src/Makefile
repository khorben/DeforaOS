TARGETS	= xmleditor
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libParser libDesktop`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libParser libDesktop`
LDFLAGS	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

xmleditor_OBJS = callbacks.o xmleditor.o main.o
xmleditor_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
xmleditor_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

xmleditor: $(xmleditor_OBJS)
	$(CC) -o xmleditor $(xmleditor_OBJS) $(xmleditor_LDFLAGS)

callbacks.o: callbacks.c callbacks.h xmleditor.h
	$(CC) $(xmleditor_CFLAGS) -c callbacks.c

xmleditor.o: xmleditor.c callbacks.h xmleditor.h ../config.h
	$(CC) $(xmleditor_CFLAGS) -c xmleditor.c

main.o: main.c
	$(CC) $(xmleditor_CFLAGS) -c main.c

clean:
	$(RM) -- $(xmleditor_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- xmleditor $(DESTDIR)$(BINDIR)/xmleditor

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/xmleditor

.PHONY: all clean distclean install uninstall
