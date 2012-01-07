PACKAGE	= Phone
VERSION	= 0.3.9
SUBDIRS	= data doc include po src tools
RM	?= rm -f
LN	?= ln -f
TAR	?= tar -czvf


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
		$(PACKAGE)-$(VERSION)/data/project.conf \
		$(PACKAGE)-$(VERSION)/data/16x16/Makefile \
		$(PACKAGE)-$(VERSION)/data/16x16/phone-dialer.png \
		$(PACKAGE)-$(VERSION)/data/16x16/phone-drafts.png \
		$(PACKAGE)-$(VERSION)/data/16x16/phone-inbox.png \
		$(PACKAGE)-$(VERSION)/data/16x16/phone-sent.png \
		$(PACKAGE)-$(VERSION)/data/16x16/project.conf \
		$(PACKAGE)-$(VERSION)/data/22x22/Makefile \
		$(PACKAGE)-$(VERSION)/data/22x22/phone-dialer.png \
		$(PACKAGE)-$(VERSION)/data/22x22/phone-signal-00.png \
		$(PACKAGE)-$(VERSION)/data/22x22/phone-signal-25.png \
		$(PACKAGE)-$(VERSION)/data/22x22/phone-signal-50.png \
		$(PACKAGE)-$(VERSION)/data/22x22/phone-signal-75.png \
		$(PACKAGE)-$(VERSION)/data/22x22/phone-signal-100.png \
		$(PACKAGE)-$(VERSION)/data/22x22/project.conf \
		$(PACKAGE)-$(VERSION)/data/24x24/Makefile \
		$(PACKAGE)-$(VERSION)/data/24x24/phone-dialer.png \
		$(PACKAGE)-$(VERSION)/data/24x24/phone-drafts.png \
		$(PACKAGE)-$(VERSION)/data/24x24/phone-inbox.png \
		$(PACKAGE)-$(VERSION)/data/24x24/phone-sent.png \
		$(PACKAGE)-$(VERSION)/data/24x24/project.conf \
		$(PACKAGE)-$(VERSION)/data/32x32/Makefile \
		$(PACKAGE)-$(VERSION)/data/32x32/phone-dialer.png \
		$(PACKAGE)-$(VERSION)/data/32x32/project.conf \
		$(PACKAGE)-$(VERSION)/data/48x48/Makefile \
		$(PACKAGE)-$(VERSION)/data/48x48/phone-dialer.png \
		$(PACKAGE)-$(VERSION)/data/48x48/phone-inbox.png \
		$(PACKAGE)-$(VERSION)/data/48x48/phone-sent.png \
		$(PACKAGE)-$(VERSION)/data/48x48/project.conf \
		$(PACKAGE)-$(VERSION)/doc/Makefile \
		$(PACKAGE)-$(VERSION)/doc/pppd-peers_phone \
		$(PACKAGE)-$(VERSION)/doc/project.conf \
		$(PACKAGE)-$(VERSION)/include/Phone.h \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/include/Phone/modem.h \
		$(PACKAGE)-$(VERSION)/include/Phone/phone.h \
		$(PACKAGE)-$(VERSION)/include/Phone/plugin.h \
		$(PACKAGE)-$(VERSION)/include/Phone/Makefile \
		$(PACKAGE)-$(VERSION)/include/Phone/project.conf \
		$(PACKAGE)-$(VERSION)/po/Makefile \
		$(PACKAGE)-$(VERSION)/po/gettext.sh \
		$(PACKAGE)-$(VERSION)/po/POTFILES \
		$(PACKAGE)-$(VERSION)/po/fr.po \
		$(PACKAGE)-$(VERSION)/po/project.conf \
		$(PACKAGE)-$(VERSION)/src/callbacks.c \
		$(PACKAGE)-$(VERSION)/src/main.c \
		$(PACKAGE)-$(VERSION)/src/modem.c \
		$(PACKAGE)-$(VERSION)/src/phone.c \
		$(PACKAGE)-$(VERSION)/src/phonectl.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/callbacks.h \
		$(PACKAGE)-$(VERSION)/src/modem.h \
		$(PACKAGE)-$(VERSION)/src/phone.h \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/modems/debug.c \
		$(PACKAGE)-$(VERSION)/src/modems/hayes.c \
		$(PACKAGE)-$(VERSION)/src/modems/purple.c \
		$(PACKAGE)-$(VERSION)/src/modems/sofia.c \
		$(PACKAGE)-$(VERSION)/src/modems/hayes.h \
		$(PACKAGE)-$(VERSION)/src/modems/Makefile \
		$(PACKAGE)-$(VERSION)/src/modems/purple.c \
		$(PACKAGE)-$(VERSION)/src/modems/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/blacklist.c \
		$(PACKAGE)-$(VERSION)/src/plugins/debug.c \
		$(PACKAGE)-$(VERSION)/src/plugins/engineering.c \
		$(PACKAGE)-$(VERSION)/src/plugins/gprs.c \
		$(PACKAGE)-$(VERSION)/src/plugins/locker.c \
		$(PACKAGE)-$(VERSION)/src/plugins/n900.c \
		$(PACKAGE)-$(VERSION)/src/plugins/openmoko.c \
		$(PACKAGE)-$(VERSION)/src/plugins/oss.c \
		$(PACKAGE)-$(VERSION)/src/plugins/panel.c \
		$(PACKAGE)-$(VERSION)/src/plugins/profiles.c \
		$(PACKAGE)-$(VERSION)/src/plugins/smscrypt.c \
		$(PACKAGE)-$(VERSION)/src/plugins/systray.c \
		$(PACKAGE)-$(VERSION)/src/plugins/ussd.c \
		$(PACKAGE)-$(VERSION)/src/plugins/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/16x16/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/16x16/phone-n900.png \
		$(PACKAGE)-$(VERSION)/src/plugins/16x16/phone-openmoko.png \
		$(PACKAGE)-$(VERSION)/src/plugins/16x16/phone-roaming.png \
		$(PACKAGE)-$(VERSION)/src/plugins/16x16/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/24x24/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/24x24/phone-n900.png \
		$(PACKAGE)-$(VERSION)/src/plugins/24x24/phone-openmoko.png \
		$(PACKAGE)-$(VERSION)/src/plugins/24x24/phone-roaming.png \
		$(PACKAGE)-$(VERSION)/src/plugins/24x24/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/32x32/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/32x32/phone-n900.png \
		$(PACKAGE)-$(VERSION)/src/plugins/32x32/phone-openmoko.png \
		$(PACKAGE)-$(VERSION)/src/plugins/32x32/phone-roaming.png \
		$(PACKAGE)-$(VERSION)/src/plugins/32x32/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/48x48/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/48x48/phone-n900.png \
		$(PACKAGE)-$(VERSION)/src/plugins/48x48/phone-openmoko.png \
		$(PACKAGE)-$(VERSION)/src/plugins/48x48/phone-roaming.png \
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
