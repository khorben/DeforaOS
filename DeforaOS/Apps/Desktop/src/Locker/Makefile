PACKAGE	= Locker
VERSION	= 0.1.3
SUBDIRS	= data include po src tools
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
		$(PACKAGE)-$(VERSION)/data/locker-settings.desktop \
		$(PACKAGE)-$(VERSION)/data/project.conf \
		$(PACKAGE)-$(VERSION)/include/Locker.h \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/include/Locker/auth.h \
		$(PACKAGE)-$(VERSION)/include/Locker/demo.h \
		$(PACKAGE)-$(VERSION)/include/Locker/locker.h \
		$(PACKAGE)-$(VERSION)/include/Locker/plugin.h \
		$(PACKAGE)-$(VERSION)/include/Locker/Makefile \
		$(PACKAGE)-$(VERSION)/include/Locker/project.conf \
		$(PACKAGE)-$(VERSION)/po/Makefile \
		$(PACKAGE)-$(VERSION)/po/gettext.sh \
		$(PACKAGE)-$(VERSION)/po/POTFILES \
		$(PACKAGE)-$(VERSION)/po/fr.po \
		$(PACKAGE)-$(VERSION)/po/project.conf \
		$(PACKAGE)-$(VERSION)/src/locker.c \
		$(PACKAGE)-$(VERSION)/src/main.c \
		$(PACKAGE)-$(VERSION)/src/lockerctl.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/locker.h \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/auth/password.c \
		$(PACKAGE)-$(VERSION)/src/auth/slider.c \
		$(PACKAGE)-$(VERSION)/src/auth/Makefile \
		$(PACKAGE)-$(VERSION)/src/auth/project.conf \
		$(PACKAGE)-$(VERSION)/src/demos/gtk-demo.c \
		$(PACKAGE)-$(VERSION)/src/demos/logo.c \
		$(PACKAGE)-$(VERSION)/src/demos/template.c \
		$(PACKAGE)-$(VERSION)/src/demos/xscreensaver.c \
		$(PACKAGE)-$(VERSION)/src/demos/xterm.c \
		$(PACKAGE)-$(VERSION)/src/demos/Makefile \
		$(PACKAGE)-$(VERSION)/src/demos/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/debug.c \
		$(PACKAGE)-$(VERSION)/src/plugins/openmoko.c \
		$(PACKAGE)-$(VERSION)/src/plugins/panel.c \
		$(PACKAGE)-$(VERSION)/src/plugins/suspend.c \
		$(PACKAGE)-$(VERSION)/src/plugins/systray.c \
		$(PACKAGE)-$(VERSION)/src/plugins/template.c \
		$(PACKAGE)-$(VERSION)/src/plugins/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/project.conf \
		$(PACKAGE)-$(VERSION)/tools/test.c \
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
