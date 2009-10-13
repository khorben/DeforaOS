PACKAGE	= utils
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
	$(RM) -r $(PACKAGE)-$(VERSION)
	$(LN) -s . $(PACKAGE)-$(VERSION)
	@$(TAR) $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/src/basename.c \
		$(PACKAGE)-$(VERSION)/src/cat.c \
		$(PACKAGE)-$(VERSION)/src/chgrp.c \
		$(PACKAGE)-$(VERSION)/src/chmod.c \
		$(PACKAGE)-$(VERSION)/src/chown.c \
		$(PACKAGE)-$(VERSION)/src/cksum.c \
		$(PACKAGE)-$(VERSION)/src/cmp.c \
		$(PACKAGE)-$(VERSION)/src/cp.c \
		$(PACKAGE)-$(VERSION)/src/df.c \
		$(PACKAGE)-$(VERSION)/src/dirname.c \
		$(PACKAGE)-$(VERSION)/src/du.c \
		$(PACKAGE)-$(VERSION)/src/echo.c \
		$(PACKAGE)-$(VERSION)/src/false.c \
		$(PACKAGE)-$(VERSION)/src/file.c \
		$(PACKAGE)-$(VERSION)/src/find.c \
		$(PACKAGE)-$(VERSION)/src/head.c \
		$(PACKAGE)-$(VERSION)/src/id.c \
		$(PACKAGE)-$(VERSION)/src/kill.c \
		$(PACKAGE)-$(VERSION)/src/link.c \
		$(PACKAGE)-$(VERSION)/src/ln.c \
		$(PACKAGE)-$(VERSION)/src/locale.c \
		$(PACKAGE)-$(VERSION)/src/logname.c \
		$(PACKAGE)-$(VERSION)/src/ls.c \
		$(PACKAGE)-$(VERSION)/src/mkdir.c \
		$(PACKAGE)-$(VERSION)/src/mkfifo.c \
		$(PACKAGE)-$(VERSION)/src/mv.c \
		$(PACKAGE)-$(VERSION)/src/nice.c \
		$(PACKAGE)-$(VERSION)/src/pr.c \
		$(PACKAGE)-$(VERSION)/src/printf.c \
		$(PACKAGE)-$(VERSION)/src/pwd.c \
		$(PACKAGE)-$(VERSION)/src/renice.c \
		$(PACKAGE)-$(VERSION)/src/rm.c \
		$(PACKAGE)-$(VERSION)/src/rmdir.c \
		$(PACKAGE)-$(VERSION)/src/sleep.c \
		$(PACKAGE)-$(VERSION)/src/strings.c \
		$(PACKAGE)-$(VERSION)/src/tail.c \
		$(PACKAGE)-$(VERSION)/src/test.c \
		$(PACKAGE)-$(VERSION)/src/time.c \
		$(PACKAGE)-$(VERSION)/src/touch.c \
		$(PACKAGE)-$(VERSION)/src/true.c \
		$(PACKAGE)-$(VERSION)/src/tty.c \
		$(PACKAGE)-$(VERSION)/src/uname.c \
		$(PACKAGE)-$(VERSION)/src/uniq.c \
		$(PACKAGE)-$(VERSION)/src/unlink.c \
		$(PACKAGE)-$(VERSION)/src/wc.c \
		$(PACKAGE)-$(VERSION)/src/who.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/common.c \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/tools/utilbox.c \
		$(PACKAGE)-$(VERSION)/tools/utils.c \
		$(PACKAGE)-$(VERSION)/tools/Makefile \
		$(PACKAGE)-$(VERSION)/tools/utils.sh \
		$(PACKAGE)-$(VERSION)/tools/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) $(PACKAGE)-$(VERSION)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean dist install uninstall
