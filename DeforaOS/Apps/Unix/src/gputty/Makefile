PACKAGE	= gputty
VERSION	= 0.9.5
SUBDIRS	= src
RM	= rm -f
PREFIX	= /usr/local
BINDIR	= $(PREFIX)/bin


all clean distclean:
	@for i in $(SUBDIRS); do make -C $$i $@ || exit $$?; done

install: all
	@for i in $(SUBDIRS); do make -C $$i PREFIX=$(PREFIX) BINDIR=$(BINDIR) $@ || exit $$?; done

dist: distclean
	$(RM) $(PACKAGE)-$(VERSION)
	ln -s . $(PACKAGE)-$(VERSION)
	tar -czf $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/*.c \
		$(PACKAGE)-$(VERSION)/src/*.h
	$(RM) $(PACKAGE)-$(VERSION)
