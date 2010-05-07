TARGETS	= contacts dialer messages phone
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic `pkg-config --cflags gtk+-2.0`
LDFLAGSF= `pkg-config --libs gtk+-2.0`
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

contacts_OBJS = contacts.o
contacts_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
contacts_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

contacts: $(contacts_OBJS)
	$(CC) -o contacts $(contacts_OBJS) $(contacts_LDFLAGS)

dialer_OBJS = dialer.o
dialer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
dialer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

dialer: $(dialer_OBJS)
	$(CC) -o dialer $(dialer_OBJS) $(dialer_LDFLAGS)

messages_OBJS = messages.o
messages_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
messages_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

messages: $(messages_OBJS)
	$(CC) -o messages $(messages_OBJS) $(messages_LDFLAGS)

phone_OBJS = callbacks.o gsm.o main.o phone.o
phone_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone: $(phone_OBJS)
	$(CC) -o phone $(phone_OBJS) $(phone_LDFLAGS)

contacts.o: contacts.c common.c
	$(CC) $(contacts_CFLAGS) -c contacts.c

dialer.o: dialer.c common.c
	$(CC) $(dialer_CFLAGS) -c dialer.c

messages.o: messages.c common.c
	$(CC) $(messages_CFLAGS) -c messages.c

callbacks.o: callbacks.c phone.h callbacks.h
	$(CC) $(phone_CFLAGS) -c callbacks.c

gsm.o: gsm.c phone.h gsm.h
	$(CC) $(phone_CFLAGS) -c gsm.c

main.o: main.c phone.h ../config.h
	$(CC) $(phone_CFLAGS) -c main.c

phone.o: phone.c gsm.h callbacks.h phone.h
	$(CC) $(phone_CFLAGS) -c phone.c

clean:
	$(RM) $(contacts_OBJS) $(dialer_OBJS) $(messages_OBJS) $(phone_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 contacts $(DESTDIR)$(BINDIR)/contacts
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 dialer $(DESTDIR)$(BINDIR)/dialer
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 messages $(DESTDIR)$(BINDIR)/messages
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 phone $(DESTDIR)$(BINDIR)/phone

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/contacts
	$(RM) $(DESTDIR)$(BINDIR)/dialer
	$(RM) $(DESTDIR)$(BINDIR)/messages
	$(RM) $(DESTDIR)$(BINDIR)/phone

.PHONY: all clean distclean install uninstall
