SUBDIRS	= auth database engines formats modules system templates
TARGETS	= daportal.php
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

daportal.php: daportal.php.in ../config.sh ../tools/subst.sh
	../tools/subst.sh -P "$(PREFIX)" -- "daportal.php"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(daportal.php_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(daportal.php_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	../tools/subst.sh -P "$(DESTDIR)$(PREFIX)/daportal/src" -i -- "daportal.php"

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	../tools/subst.sh -P "$(DESTDIR)$(PREFIX)/daportal/src" -u -- "daportal.php"

.PHONY: all subdirs clean distclean install uninstall
