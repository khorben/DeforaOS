SUBDIRS	= css icons js themes
TARGETS	= index.php
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

index.php: index.php.in ../config.sh ../tools/subst.sh
	../tools/subst.sh -P "$(PREFIX)" -- "index.php"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(index.php_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(index.php_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	../tools/subst.sh -P "$(DESTDIR)$(PREFIX)/daportal/data" -i -- "index.php"

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	../tools/subst.sh -P "$(DESTDIR)$(PREFIX)/daportal/data" -u -- "index.php"

.PHONY: all subdirs clean distclean install uninstall
