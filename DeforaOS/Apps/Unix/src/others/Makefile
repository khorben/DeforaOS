PACKAGE	= others
VERSION	= 0.0.2
SUBDIRS	= src tools
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
		$(PACKAGE)-$(VERSION)/src/halt.c \
		$(PACKAGE)-$(VERSION)/src/hexdump.c \
		$(PACKAGE)-$(VERSION)/src/host.c \
		$(PACKAGE)-$(VERSION)/src/hostname.c \
		$(PACKAGE)-$(VERSION)/src/ifconfig.c \
		$(PACKAGE)-$(VERSION)/src/login.c \
		$(PACKAGE)-$(VERSION)/src/mktemp.c \
		$(PACKAGE)-$(VERSION)/src/mount.c \
		$(PACKAGE)-$(VERSION)/src/poweroff.c \
		$(PACKAGE)-$(VERSION)/src/reboot.c \
		$(PACKAGE)-$(VERSION)/src/tar.c \
		$(PACKAGE)-$(VERSION)/src/umount.c \
		$(PACKAGE)-$(VERSION)/src/uptime.c \
		$(PACKAGE)-$(VERSION)/src/w.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/ifconfig.c \
		$(PACKAGE)-$(VERSION)/src/tar.h \
		$(PACKAGE)-$(VERSION)/src/utmpx.c \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/tools/linuxrc.c \
		$(PACKAGE)-$(VERSION)/tools/oinit.c \
		$(PACKAGE)-$(VERSION)/tools/otherbox.c \
		$(PACKAGE)-$(VERSION)/tools/others.c \
		$(PACKAGE)-$(VERSION)/tools/Makefile \
		$(PACKAGE)-$(VERSION)/tools/others.sh \
		$(PACKAGE)-$(VERSION)/tools/otherbox.h \
		$(PACKAGE)-$(VERSION)/tools/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) -- $(PACKAGE)-$(VERSION)

install:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean dist install uninstall
