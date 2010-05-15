SUBDIRS	= plugins
TARGETS	= phone phone-contacts phone-dialer phone-messages
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic `pkg-config --cflags gtk+-2.0`
LDFLAGSF= `pkg-config --libs gtk+-2.0` -lSystem
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

phone_OBJS = callbacks.o command.o gsm.o main.o modem.o phone.o
phone_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone: $(phone_OBJS)
	$(CC) -o phone $(phone_OBJS) $(phone_LDFLAGS)

phone-contacts_OBJS = contacts.o
phone-contacts_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone-contacts_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone-contacts: $(phone-contacts_OBJS)
	$(CC) -o phone-contacts $(phone-contacts_OBJS) $(phone-contacts_LDFLAGS)

phone-dialer_OBJS = dialer.o
phone-dialer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone-dialer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone-dialer: $(phone-dialer_OBJS)
	$(CC) -o phone-dialer $(phone-dialer_OBJS) $(phone-dialer_LDFLAGS)

phone-messages_OBJS = messages.o
phone-messages_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone-messages_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone-messages: $(phone-messages_OBJS)
	$(CC) -o phone-messages $(phone-messages_OBJS) $(phone-messages_LDFLAGS)

callbacks.o: callbacks.c ../include/Phone.h phone.h callbacks.h
	$(CC) $(phone_CFLAGS) -c callbacks.c

command.o: command.c gsm.h command.h
	$(CC) $(phone_CFLAGS) -c command.c

gsm.o: gsm.c ../include/Phone.h phone.h command.h modem.h gsm.h
	$(CC) $(phone_CFLAGS) -c gsm.c

main.o: main.c ../include/Phone.h phone.h ../config.h
	$(CC) $(phone_CFLAGS) -c main.c

modem.o: modem.c gsm.h modem.h
	$(CC) $(phone_CFLAGS) -c modem.c

phone.o: phone.c ../include/Phone.h phone.h gsm.h callbacks.h ../config.h
	$(CC) $(phone_CFLAGS) -c phone.c

contacts.o: contacts.c common.c
	$(CC) $(phone-contacts_CFLAGS) -c contacts.c

dialer.o: dialer.c common.c
	$(CC) $(phone-dialer_CFLAGS) -c dialer.c

messages.o: messages.c common.c
	$(CC) $(phone-messages_CFLAGS) -c messages.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) $(phone_OBJS) $(phone-contacts_OBJS) $(phone-dialer_OBJS) $(phone-messages_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) $(phone_OBJS) $(phone-contacts_OBJS) $(phone-dialer_OBJS) $(phone-messages_OBJS)
	$(RM) $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 phone $(DESTDIR)$(BINDIR)/phone
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 phone-contacts $(DESTDIR)$(BINDIR)/phone-contacts
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 phone-dialer $(DESTDIR)$(BINDIR)/phone-dialer
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 phone-messages $(DESTDIR)$(BINDIR)/phone-messages

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) $(DESTDIR)$(BINDIR)/phone
	$(RM) $(DESTDIR)$(BINDIR)/phone-contacts
	$(RM) $(DESTDIR)$(BINDIR)/phone-dialer
	$(RM) $(DESTDIR)$(BINDIR)/phone-messages

.PHONY: all subdirs clean distclean install uninstall
