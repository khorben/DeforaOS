TARGETS	= AppBroker
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= -lSystem -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
LDFLAGS	= -L../src
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

AppBroker_OBJS = appbroker.o
AppBroker_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
AppBroker_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

AppBroker: $(AppBroker_OBJS)
	$(CC) -o AppBroker $(AppBroker_OBJS) $(AppBroker_LDFLAGS)

appbroker.o: appbroker.c
	$(CC) $(AppBroker_CFLAGS) -c appbroker.c

clean:
	$(RM) $(AppBroker_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 AppBroker $(DESTDIR)$(BINDIR)/AppBroker

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/AppBroker

.PHONY: all clean distclean install uninstall
