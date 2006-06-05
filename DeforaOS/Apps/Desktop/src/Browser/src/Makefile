TARGETS	= browser
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

browser_OBJS = browser.o
browser_CFLAGS = $(CFLAGSF) $(CFLAGS)
browser: $(browser_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o browser $(browser_OBJS)

browser.o: browser.c browser.h
	$(CC) $(browser_CFLAGS) -c browser.c

clean:
	$(RM) $(browser_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 browser $(DESTDIR)$(BINDIR)/browser

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/browser
