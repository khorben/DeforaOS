SUBDIRS	= modems plugins
TARGETS	= phone phone-contacts phone-dialer phone-logs phone-messages phone-resume phone-settings phone-suspend
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS= 
CFLAGSF	= -W `pkg-config --cflags libSystem gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem gtk+-2.0`
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

phone_OBJS = callbacks.o main.o modem.o phone.o
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

phone-logs_OBJS = logs.o
phone-logs_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone-logs_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone-logs: $(phone-logs_OBJS)
	$(CC) -o phone-logs $(phone-logs_OBJS) $(phone-logs_LDFLAGS)

phone-messages_OBJS = messages.o
phone-messages_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone-messages_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone-messages: $(phone-messages_OBJS)
	$(CC) -o phone-messages $(phone-messages_OBJS) $(phone-messages_LDFLAGS)

phone-resume_OBJS = resume.o
phone-resume_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone-resume_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone-resume: $(phone-resume_OBJS)
	$(CC) -o phone-resume $(phone-resume_OBJS) $(phone-resume_LDFLAGS)

phone-settings_OBJS = settings.o
phone-settings_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone-settings_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone-settings: $(phone-settings_OBJS)
	$(CC) -o phone-settings $(phone-settings_OBJS) $(phone-settings_LDFLAGS)

phone-suspend_OBJS = suspend.o
phone-suspend_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone-suspend_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone-suspend: $(phone-suspend_OBJS)
	$(CC) -o phone-suspend $(phone-suspend_OBJS) $(phone-suspend_LDFLAGS)

callbacks.o: callbacks.c ../include/Phone/phone.h phone.h callbacks.h
	$(CC) $(phone_CFLAGS) -c callbacks.c

main.o: main.c ../include/Phone/phone.h phone.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(phone_CFLAGS) -c main.c

modem.o: modem.c modem.h ../config.h
	$(CC) $(phone_CFLAGS) -c modem.c

phone.o: phone.c ../include/Phone/phone.h modem.h phone.h callbacks.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(phone_CFLAGS) -c phone.c

contacts.o: contacts.c common.c
	$(CC) $(phone-contacts_CFLAGS) -c contacts.c

dialer.o: dialer.c common.c
	$(CC) $(phone-dialer_CFLAGS) -c dialer.c

logs.o: logs.c common.c
	$(CC) $(phone-logs_CFLAGS) -c logs.c

messages.o: messages.c common.c
	$(CC) $(phone-messages_CFLAGS) -c messages.c

resume.o: resume.c common.c
	$(CC) $(phone-resume_CFLAGS) -c resume.c

settings.o: settings.c common.c
	$(CC) $(phone-settings_CFLAGS) -c settings.c

suspend.o: suspend.c common.c
	$(CC) $(phone-suspend_CFLAGS) -c suspend.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(phone_OBJS) $(phone-contacts_OBJS) $(phone-dialer_OBJS) $(phone-logs_OBJS) $(phone-messages_OBJS) $(phone-resume_OBJS) $(phone-settings_OBJS) $(phone-suspend_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(phone_OBJS) $(phone-contacts_OBJS) $(phone-dialer_OBJS) $(phone-logs_OBJS) $(phone-messages_OBJS) $(phone-resume_OBJS) $(phone-settings_OBJS) $(phone-suspend_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- phone $(DESTDIR)$(BINDIR)/phone
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- phone-contacts $(DESTDIR)$(BINDIR)/phone-contacts
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- phone-dialer $(DESTDIR)$(BINDIR)/phone-dialer
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- phone-logs $(DESTDIR)$(BINDIR)/phone-logs
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- phone-messages $(DESTDIR)$(BINDIR)/phone-messages
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- phone-resume $(DESTDIR)$(BINDIR)/phone-resume
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- phone-settings $(DESTDIR)$(BINDIR)/phone-settings
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- phone-suspend $(DESTDIR)$(BINDIR)/phone-suspend

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(BINDIR)/phone
	$(RM) -- $(DESTDIR)$(BINDIR)/phone-contacts
	$(RM) -- $(DESTDIR)$(BINDIR)/phone-dialer
	$(RM) -- $(DESTDIR)$(BINDIR)/phone-logs
	$(RM) -- $(DESTDIR)$(BINDIR)/phone-messages
	$(RM) -- $(DESTDIR)$(BINDIR)/phone-resume
	$(RM) -- $(DESTDIR)$(BINDIR)/phone-settings
	$(RM) -- $(DESTDIR)$(BINDIR)/phone-suspend

.PHONY: all subdirs clean distclean install uninstall
