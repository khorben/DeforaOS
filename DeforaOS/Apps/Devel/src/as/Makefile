PACKAGE	= as
VERSION	= 0.1.0
SUBDIRS	= src include
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
		$(PACKAGE)-$(VERSION)/src/as.c \
		$(PACKAGE)-$(VERSION)/src/code.c \
		$(PACKAGE)-$(VERSION)/src/parser.c \
		$(PACKAGE)-$(VERSION)/src/token.c \
		$(PACKAGE)-$(VERSION)/src/arch/arch.c \
		$(PACKAGE)-$(VERSION)/src/format/format.c \
		$(PACKAGE)-$(VERSION)/src/main.c \
		$(PACKAGE)-$(VERSION)/src/disas.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/code.h \
		$(PACKAGE)-$(VERSION)/src/common.h \
		$(PACKAGE)-$(VERSION)/src/parser.h \
		$(PACKAGE)-$(VERSION)/src/token.h \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/arch/amd64.c \
		$(PACKAGE)-$(VERSION)/src/arch/i386.c \
		$(PACKAGE)-$(VERSION)/src/arch/i386_real.c \
		$(PACKAGE)-$(VERSION)/src/arch/i486.c \
		$(PACKAGE)-$(VERSION)/src/arch/i586.c \
		$(PACKAGE)-$(VERSION)/src/arch/i686.c \
		$(PACKAGE)-$(VERSION)/src/arch/java.c \
		$(PACKAGE)-$(VERSION)/src/arch/sparc.c \
		$(PACKAGE)-$(VERSION)/src/arch/sparc64.c \
		$(PACKAGE)-$(VERSION)/src/arch/Makefile \
		$(PACKAGE)-$(VERSION)/src/arch/arch.h \
		$(PACKAGE)-$(VERSION)/src/arch/80386.ins \
		$(PACKAGE)-$(VERSION)/src/arch/80386.reg \
		$(PACKAGE)-$(VERSION)/src/arch/80486.ins \
		$(PACKAGE)-$(VERSION)/src/arch/80486.reg \
		$(PACKAGE)-$(VERSION)/src/arch/80586.ins \
		$(PACKAGE)-$(VERSION)/src/arch/80586.reg \
		$(PACKAGE)-$(VERSION)/src/arch/80686.ins \
		$(PACKAGE)-$(VERSION)/src/arch/80686.reg \
		$(PACKAGE)-$(VERSION)/src/arch/amd64.ins \
		$(PACKAGE)-$(VERSION)/src/arch/amd64.reg \
		$(PACKAGE)-$(VERSION)/src/arch/sparc.ins \
		$(PACKAGE)-$(VERSION)/src/arch/sparc.reg \
		$(PACKAGE)-$(VERSION)/src/arch/sparc64.ins \
		$(PACKAGE)-$(VERSION)/src/arch/sparc64.reg \
		$(PACKAGE)-$(VERSION)/src/arch/common.reg \
		$(PACKAGE)-$(VERSION)/src/arch/null.ins \
		$(PACKAGE)-$(VERSION)/src/arch/project.conf \
		$(PACKAGE)-$(VERSION)/src/format/elf.c \
		$(PACKAGE)-$(VERSION)/src/format/flat.c \
		$(PACKAGE)-$(VERSION)/src/format/java.c \
		$(PACKAGE)-$(VERSION)/src/format/Makefile \
		$(PACKAGE)-$(VERSION)/src/format/format.h \
		$(PACKAGE)-$(VERSION)/src/format/project.conf \
		$(PACKAGE)-$(VERSION)/include/as.h \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/config.h \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) -- $(PACKAGE)-$(VERSION)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean dist install uninstall
