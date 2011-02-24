PACKAGE	= Panel
VERSION	= 0.2.1
SUBDIRS	= data include po src
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
		$(PACKAGE)-$(VERSION)/data/panel-settings.desktop \
		$(PACKAGE)-$(VERSION)/data/project.conf \
		$(PACKAGE)-$(VERSION)/data/16x16/Makefile \
		$(PACKAGE)-$(VERSION)/data/16x16/panel-applet-bluetooth.png \
		$(PACKAGE)-$(VERSION)/data/16x16/panel-applet-desktop.png \
		$(PACKAGE)-$(VERSION)/data/16x16/panel-settings.png \
		$(PACKAGE)-$(VERSION)/data/16x16/project.conf \
		$(PACKAGE)-$(VERSION)/data/22x22/Makefile \
		$(PACKAGE)-$(VERSION)/data/22x22/panel-applet-bluetooth.png \
		$(PACKAGE)-$(VERSION)/data/22x22/panel-applet-desktop.png \
		$(PACKAGE)-$(VERSION)/data/22x22/panel-settings.png \
		$(PACKAGE)-$(VERSION)/data/22x22/project.conf \
		$(PACKAGE)-$(VERSION)/data/24x24/Makefile \
		$(PACKAGE)-$(VERSION)/data/24x24/panel-applet-bluetooth.png \
		$(PACKAGE)-$(VERSION)/data/24x24/panel-applet-desktop.png \
		$(PACKAGE)-$(VERSION)/data/24x24/panel-settings.png \
		$(PACKAGE)-$(VERSION)/data/24x24/project.conf \
		$(PACKAGE)-$(VERSION)/data/32x32/Makefile \
		$(PACKAGE)-$(VERSION)/data/32x32/panel-applet-bluetooth.png \
		$(PACKAGE)-$(VERSION)/data/32x32/panel-applet-desktop.png \
		$(PACKAGE)-$(VERSION)/data/32x32/panel-settings.png \
		$(PACKAGE)-$(VERSION)/data/32x32/project.conf \
		$(PACKAGE)-$(VERSION)/data/48x48/Makefile \
		$(PACKAGE)-$(VERSION)/data/48x48/panel-applet-bluetooth.png \
		$(PACKAGE)-$(VERSION)/data/48x48/panel-applet-desktop.png \
		$(PACKAGE)-$(VERSION)/data/48x48/panel-settings.png \
		$(PACKAGE)-$(VERSION)/data/48x48/project.conf \
		$(PACKAGE)-$(VERSION)/data/scalable/Makefile \
		$(PACKAGE)-$(VERSION)/data/scalable/panel-applet-bluetooth.svg \
		$(PACKAGE)-$(VERSION)/data/scalable/project.conf \
		$(PACKAGE)-$(VERSION)/include/Panel.h \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/po/Makefile \
		$(PACKAGE)-$(VERSION)/po/gettext.sh \
		$(PACKAGE)-$(VERSION)/po/POTFILES \
		$(PACKAGE)-$(VERSION)/po/fr.po \
		$(PACKAGE)-$(VERSION)/po/project.conf \
		$(PACKAGE)-$(VERSION)/src/panel.c \
		$(PACKAGE)-$(VERSION)/src/window.c \
		$(PACKAGE)-$(VERSION)/src/main.c \
		$(PACKAGE)-$(VERSION)/src/settings.c \
		$(PACKAGE)-$(VERSION)/src/run.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/common.h \
		$(PACKAGE)-$(VERSION)/src/window.h \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/applets/battery.c \
		$(PACKAGE)-$(VERSION)/src/applets/bluetooth.c \
		$(PACKAGE)-$(VERSION)/src/applets/clock.c \
		$(PACKAGE)-$(VERSION)/src/applets/cpu.c \
		$(PACKAGE)-$(VERSION)/src/applets/cpufreq.c \
		$(PACKAGE)-$(VERSION)/src/applets/desktop.c \
		$(PACKAGE)-$(VERSION)/src/applets/gps.c \
		$(PACKAGE)-$(VERSION)/src/applets/gsm.c \
		$(PACKAGE)-$(VERSION)/src/applets/keyboard.c \
		$(PACKAGE)-$(VERSION)/src/applets/lock.c \
		$(PACKAGE)-$(VERSION)/src/applets/logout.c \
		$(PACKAGE)-$(VERSION)/src/applets/main.c \
		$(PACKAGE)-$(VERSION)/src/applets/memory.c \
		$(PACKAGE)-$(VERSION)/src/applets/pager.c \
		$(PACKAGE)-$(VERSION)/src/applets/phone.c \
		$(PACKAGE)-$(VERSION)/src/applets/swap.c \
		$(PACKAGE)-$(VERSION)/src/applets/systray.c \
		$(PACKAGE)-$(VERSION)/src/applets/tasks.c \
		$(PACKAGE)-$(VERSION)/src/applets/volume.c \
		$(PACKAGE)-$(VERSION)/src/applets/wpa_supplicant.c \
		$(PACKAGE)-$(VERSION)/src/applets/Makefile \
		$(PACKAGE)-$(VERSION)/src/applets/tasks.atoms \
		$(PACKAGE)-$(VERSION)/src/applets/project.conf \
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
