PACKAGE	= configure
VERSION	= 0.0.1
SUBDIRS	= src
PREFIX	= /usr/local
BINDIR	= $(PREFIX)/bin


all clean distclean:
	@for i in $(SUBDIRS); do make -C $$i $@ || exit $$?; done

install: all
	@for i in $(SUBDIRS); do make -C $$i PREFIX=$(PREFIX) BINDIR=$(BINDIR) $@ || exit $$?; done

dist: distclean
	$(RM) -r $(PACKAGE)-$(VERSION)
	ln -s . $(PACKAGE)-$(VERSION)
	tar -czf $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/AUTHORS \
		$(PACKAGE)-$(VERSION)/BUGS \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/INSTALL \
		$(PACKAGE)-$(VERSION)/README \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/*.c
	$(RM) $(PACKAGE)-$(VERSION)
