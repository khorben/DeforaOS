PACKAGE	= scripts
VERSION	= 0.0.0
SUBDIRS	= src targets
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
	$(RM) -r $(PACKAGE)-$(VERSION)
	$(LN) -s . $(PACKAGE)-$(VERSION)
	@$(TAR) $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/deforaos-build.sh \
		$(PACKAGE)-$(VERSION)/src/deforaos-update.sh \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/targets/Makefile \
		$(PACKAGE)-$(VERSION)/targets/Linux \
		$(PACKAGE)-$(VERSION)/targets/Linux-arm \
		$(PACKAGE)-$(VERSION)/targets/Linux-i386 \
		$(PACKAGE)-$(VERSION)/targets/NetBSD \
		$(PACKAGE)-$(VERSION)/targets/NetBSD-amd64 \
		$(PACKAGE)-$(VERSION)/targets/NetBSD-i386 \
		$(PACKAGE)-$(VERSION)/targets/NetBSD-sparc \
		$(PACKAGE)-$(VERSION)/targets/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) $(PACKAGE)-$(VERSION)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean dist install uninstall
