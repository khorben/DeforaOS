PACKAGE	= as
VERSION	= 0.0.0
SUBDIRS	= src
LN	= ln -sf
TAR	= tar -czvf


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

dist:
	$(RM) $(PACKAGE)-$(VERSION)
	$(LN) . $(PACKAGE)-$(VERSION)
	@$(TAR) $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/src/format/elf.c \
		$(PACKAGE)-$(VERSION)/src/format/flat.c \
		$(PACKAGE)-$(VERSION)/src/format/format.h \
		$(PACKAGE)-$(VERSION)/src/format/project.conf \
		$(PACKAGE)-$(VERSION)/src/format/Makefile \
		$(PACKAGE)-$(VERSION)/src/arch/i386.c \
		$(PACKAGE)-$(VERSION)/src/arch/i486.c \
		$(PACKAGE)-$(VERSION)/src/arch/sparc.c \
		$(PACKAGE)-$(VERSION)/src/arch/arch.h \
		$(PACKAGE)-$(VERSION)/src/arch/common.reg \
		$(PACKAGE)-$(VERSION)/src/arch/80386.reg \
		$(PACKAGE)-$(VERSION)/src/arch/80386.ins \
		$(PACKAGE)-$(VERSION)/src/arch/80486.reg \
		$(PACKAGE)-$(VERSION)/src/arch/80486.ins \
		$(PACKAGE)-$(VERSION)/src/arch/project.conf \
		$(PACKAGE)-$(VERSION)/src/arch/Makefile \
		$(PACKAGE)-$(VERSION)/src/as.c \
		$(PACKAGE)-$(VERSION)/src/code.c \
		$(PACKAGE)-$(VERSION)/src/parser.c \
		$(PACKAGE)-$(VERSION)/src/scanner.c \
		$(PACKAGE)-$(VERSION)/src/token.c \
		$(PACKAGE)-$(VERSION)/src/arch/arch.c \
		$(PACKAGE)-$(VERSION)/src/format/format.c \
		$(PACKAGE)-$(VERSION)/src/as.h \
		$(PACKAGE)-$(VERSION)/src/code.h \
		$(PACKAGE)-$(VERSION)/src/parser.h \
		$(PACKAGE)-$(VERSION)/src/scanner.h \
		$(PACKAGE)-$(VERSION)/src/token.h \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile
	$(RM) $(PACKAGE)-$(VERSION)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
