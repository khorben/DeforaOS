PACKAGE	= Phone
VERSION	= 0.2.9
SUBDIRS	= data include po src tools
RM	= rm -f
LN	= ln -f
TAR	= tar -czvf


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

dist:
	$(RM) -r -- $(PACKAGE)-$(VERSION)
	$(LN) -s -- . $(PACKAGE)-$(VERSION)
	@$(TAR) $(PACKAGE)-$(VERSION).tar.gz -- \
		$(PACKAGE)-$(VERSION)/data/Makefile \
		$(PACKAGE)-$(VERSION)/data/phone-contacts.desktop \
		$(PACKAGE)-$(VERSION)/data/phone-dialer.desktop \
		$(PACKAGE)-$(VERSION)/data/phone-log.desktop \
		$(PACKAGE)-$(VERSION)/data/phone-messages.desktop \
		$(PACKAGE)-$(VERSION)/data/phone-settings.desktop \
		$(PACKAGE)-$(VERSION)/data/phone-signal-00.png \
		$(PACKAGE)-$(VERSION)/data/phone-signal-25.png \
		$(PACKAGE)-$(VERSION)/data/phone-signal-50.png \
		$(PACKAGE)-$(VERSION)/data/phone-signal-75.png \
		$(PACKAGE)-$(VERSION)/data/phone-signal-100.png \
		$(PACKAGE)-$(VERSION)/data/project.conf \
		$(PACKAGE)-$(VERSION)/data/16x16/Makefile \
		$(PACKAGE)-$(VERSION)/data/16x16/phone-dialer.png \
		$(PACKAGE)-$(VERSION)/data/16x16/project.conf \
		$(PACKAGE)-$(VERSION)/data/22x22/Makefile \
		$(PACKAGE)-$(VERSION)/data/22x22/phone-dialer.png \
		$(PACKAGE)-$(VERSION)/data/22x22/project.conf \
		$(PACKAGE)-$(VERSION)/data/24x24/Makefile \
		$(PACKAGE)-$(VERSION)/data/24x24/phone-dialer.png \
		$(PACKAGE)-$(VERSION)/data/24x24/project.conf \
		$(PACKAGE)-$(VERSION)/data/32x32/Makefile \
		$(PACKAGE)-$(VERSION)/data/32x32/phone-dialer.png \
		$(PACKAGE)-$(VERSION)/data/32x32/project.conf \
		$(PACKAGE)-$(VERSION)/data/48x48/Makefile \
		$(PACKAGE)-$(VERSION)/data/48x48/phone-dialer.png \
		$(PACKAGE)-$(VERSION)/data/48x48/project.conf \
		$(PACKAGE)-$(VERSION)/include/Phone.h \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/po/Makefile \
		$(PACKAGE)-$(VERSION)/po/gettext.sh \
		$(PACKAGE)-$(VERSION)/po/POTFILES \
		$(PACKAGE)-$(VERSION)/po/fr.po \
		$(PACKAGE)-$(VERSION)/po/project.conf \
		$(PACKAGE)-$(VERSION)/src/callbacks.c \
		$(PACKAGE)-$(VERSION)/src/command.c \
		$(PACKAGE)-$(VERSION)/src/gsm.c \
		$(PACKAGE)-$(VERSION)/src/main.c \
		$(PACKAGE)-$(VERSION)/src/modem.c \
		$(PACKAGE)-$(VERSION)/src/phone.c \
		$(PACKAGE)-$(VERSION)/src/contacts.c \
		$(PACKAGE)-$(VERSION)/src/dialer.c \
		$(PACKAGE)-$(VERSION)/src/logs.c \
		$(PACKAGE)-$(VERSION)/src/messages.c \
		$(PACKAGE)-$(VERSION)/src/resume.c \
		$(PACKAGE)-$(VERSION)/src/settings.c \
		$(PACKAGE)-$(VERSION)/src/suspend.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/callbacks.h \
		$(PACKAGE)-$(VERSION)/src/command.h \
		$(PACKAGE)-$(VERSION)/src/gsm.h \
		$(PACKAGE)-$(VERSION)/src/modem.h \
		$(PACKAGE)-$(VERSION)/src/phone.h \
		$(PACKAGE)-$(VERSION)/src/common.c \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/blacklist.c \
		$(PACKAGE)-$(VERSION)/src/plugins/debug.c \
		$(PACKAGE)-$(VERSION)/src/plugins/engineering.c \
		$(PACKAGE)-$(VERSION)/src/plugins/gprs.c \
		$(PACKAGE)-$(VERSION)/src/plugins/openmoko.c \
		$(PACKAGE)-$(VERSION)/src/plugins/oss.c \
		$(PACKAGE)-$(VERSION)/src/plugins/panel.c \
		$(PACKAGE)-$(VERSION)/src/plugins/profiles.c \
		$(PACKAGE)-$(VERSION)/src/plugins/smscrypt.c \
		$(PACKAGE)-$(VERSION)/src/plugins/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/16x16/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/16x16/phone-openmoko.png \
		$(PACKAGE)-$(VERSION)/src/plugins/16x16/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/24x24/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/24x24/phone-openmoko.png \
		$(PACKAGE)-$(VERSION)/src/plugins/24x24/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/32x32/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/32x32/phone-openmoko.png \
		$(PACKAGE)-$(VERSION)/src/plugins/32x32/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/48x48/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/48x48/phone-openmoko.png \
		$(PACKAGE)-$(VERSION)/src/plugins/48x48/project.conf \
		$(PACKAGE)-$(VERSION)/tools/engineering.c \
		$(PACKAGE)-$(VERSION)/tools/pdu.c \
		$(PACKAGE)-$(VERSION)/tools/smscrypt.c \
		$(PACKAGE)-$(VERSION)/tools/Makefile \
		$(PACKAGE)-$(VERSION)/tools/project.conf \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/config.h \
		$(PACKAGE)-$(VERSION)/config.sh \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) -- $(PACKAGE)-$(VERSION)

install:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean dist install uninstall
