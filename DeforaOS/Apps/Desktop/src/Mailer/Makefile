PACKAGE	= Mailer
VERSION	= 0.1.4
SUBDIRS	= data doc include po src
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
		$(PACKAGE)-$(VERSION)/data/Mailer.pc.in \
		$(PACKAGE)-$(VERSION)/data/mailer.desktop \
		$(PACKAGE)-$(VERSION)/data/pkgconfig.sh \
		$(PACKAGE)-$(VERSION)/data/project.conf \
		$(PACKAGE)-$(VERSION)/data/16x16/Makefile \
		$(PACKAGE)-$(VERSION)/data/16x16/mailer.png \
		$(PACKAGE)-$(VERSION)/data/16x16/mailer-accounts.png \
		$(PACKAGE)-$(VERSION)/data/16x16/mailer-drafts.png \
		$(PACKAGE)-$(VERSION)/data/16x16/mailer-inbox.png \
		$(PACKAGE)-$(VERSION)/data/16x16/mailer-outbox.png \
		$(PACKAGE)-$(VERSION)/data/16x16/mailer-sent.png \
		$(PACKAGE)-$(VERSION)/data/16x16/project.conf \
		$(PACKAGE)-$(VERSION)/data/24x24/Makefile \
		$(PACKAGE)-$(VERSION)/data/24x24/mailer.png \
		$(PACKAGE)-$(VERSION)/data/24x24/mailer-accounts.png \
		$(PACKAGE)-$(VERSION)/data/24x24/mailer-drafts.png \
		$(PACKAGE)-$(VERSION)/data/24x24/mailer-inbox.png \
		$(PACKAGE)-$(VERSION)/data/24x24/mailer-outbox.png \
		$(PACKAGE)-$(VERSION)/data/24x24/mailer-sent.png \
		$(PACKAGE)-$(VERSION)/data/24x24/project.conf \
		$(PACKAGE)-$(VERSION)/data/48x48/Makefile \
		$(PACKAGE)-$(VERSION)/data/48x48/mailer.png \
		$(PACKAGE)-$(VERSION)/data/48x48/mailer-inbox.png \
		$(PACKAGE)-$(VERSION)/data/48x48/mailer-outbox.png \
		$(PACKAGE)-$(VERSION)/data/48x48/mailer-sent.png \
		$(PACKAGE)-$(VERSION)/data/48x48/project.conf \
		$(PACKAGE)-$(VERSION)/doc/Makefile \
		$(PACKAGE)-$(VERSION)/doc/GRAMMAR \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc.sh \
		$(PACKAGE)-$(VERSION)/doc/project.conf \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/Makefile \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/Mailer-docs.xml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/project.conf \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/Makefile \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/Mailer.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/project.conf \
		$(PACKAGE)-$(VERSION)/include/Mailer.h \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/include/Mailer/account.h \
		$(PACKAGE)-$(VERSION)/include/Mailer/folder.h \
		$(PACKAGE)-$(VERSION)/include/Mailer/helper.h \
		$(PACKAGE)-$(VERSION)/include/Mailer/mailer.h \
		$(PACKAGE)-$(VERSION)/include/Mailer/message.h \
		$(PACKAGE)-$(VERSION)/include/Mailer/plugin.h \
		$(PACKAGE)-$(VERSION)/include/Mailer/transport.h \
		$(PACKAGE)-$(VERSION)/include/Mailer/Makefile \
		$(PACKAGE)-$(VERSION)/include/Mailer/project.conf \
		$(PACKAGE)-$(VERSION)/po/Makefile \
		$(PACKAGE)-$(VERSION)/po/gettext.sh \
		$(PACKAGE)-$(VERSION)/po/POTFILES \
		$(PACKAGE)-$(VERSION)/po/fr.po \
		$(PACKAGE)-$(VERSION)/po/project.conf \
		$(PACKAGE)-$(VERSION)/src/account.c \
		$(PACKAGE)-$(VERSION)/src/callbacks.c \
		$(PACKAGE)-$(VERSION)/src/compose.c \
		$(PACKAGE)-$(VERSION)/src/folder.c \
		$(PACKAGE)-$(VERSION)/src/helper.c \
		$(PACKAGE)-$(VERSION)/src/mailer.c \
		$(PACKAGE)-$(VERSION)/src/message.c \
		$(PACKAGE)-$(VERSION)/src/compose-main.c \
		$(PACKAGE)-$(VERSION)/src/main.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/account.h \
		$(PACKAGE)-$(VERSION)/src/callbacks.h \
		$(PACKAGE)-$(VERSION)/src/common.c \
		$(PACKAGE)-$(VERSION)/src/compose.h \
		$(PACKAGE)-$(VERSION)/src/folder.h \
		$(PACKAGE)-$(VERSION)/src/mailer.h \
		$(PACKAGE)-$(VERSION)/src/message.h \
		$(PACKAGE)-$(VERSION)/src/gtkassistant.c \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/account/imap4.c \
		$(PACKAGE)-$(VERSION)/src/account/mbox.c \
		$(PACKAGE)-$(VERSION)/src/account/pop3.c \
		$(PACKAGE)-$(VERSION)/src/account/nntp.c \
		$(PACKAGE)-$(VERSION)/src/account/rss.c \
		$(PACKAGE)-$(VERSION)/src/account/Makefile \
		$(PACKAGE)-$(VERSION)/src/account/project.conf \
		$(PACKAGE)-$(VERSION)/src/plugins/mailing-lists.c \
		$(PACKAGE)-$(VERSION)/src/plugins/search.c \
		$(PACKAGE)-$(VERSION)/src/plugins/Makefile \
		$(PACKAGE)-$(VERSION)/src/plugins/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/config.h \
		$(PACKAGE)-$(VERSION)/config.sh \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) -- $(PACKAGE)-$(VERSION)

install:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean dist install uninstall
