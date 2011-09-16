TARGETS	= AppBroker AppClient
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -W `pkg-config --cflags libSystem`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= -L../src `pkg-config --libs libSystem` -lApp
LDFLAGS	= -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
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

AppClient_OBJS = appclient.o
AppClient_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
AppClient_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

AppClient: $(AppClient_OBJS)
	$(CC) -o AppClient $(AppClient_OBJS) $(AppClient_LDFLAGS)

appbroker.o: appbroker.c
	$(CC) $(AppBroker_CFLAGS) -c appbroker.c

appclient.o: appclient.c
	$(CC) $(AppClient_CFLAGS) -c appclient.c

clean:
	$(RM) -- $(AppBroker_OBJS) $(AppClient_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- AppBroker $(DESTDIR)$(BINDIR)/AppBroker

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/AppBroker

.PHONY: all clean distclean install uninstall
