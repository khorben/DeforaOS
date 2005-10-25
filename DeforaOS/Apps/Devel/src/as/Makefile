PACKAGE	= as
VERSION	= 0.0.0
SUBDIRS	= src
TAR	= tar cfzv


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

dist: distclean
	@$(TAR) $(PACKAGE)-$(VERSION).tar.gz \
		src/format/elf.c \
		src/format/flat.c \
		src/format/format.h \
		src/format/project.conf \
		src/format/Makefile \
		src/arch/i386.c \
		src/arch/i486.c \
		src/arch/sparc.c \
		src/arch/arch.h \
		src/arch/common.reg \
		src/arch/80386.reg \
		src/arch/80386.ins \
		src/arch/80486.reg \
		src/arch/80486.ins \
		src/arch/project.conf \
		src/arch/Makefile \
		src/as.c \
		src/code.c \
		src/parser.c \
		src/scanner.c \
		src/token.c \
		src/arch/arch.c \
		src/format/format.c \
		src/as.h \
		src/code.h \
		src/parser.h \
		src/scanner.h \
		src/token.h \
		src/project.conf \
		src/Makefile \
		project.conf \
		Makefile

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
