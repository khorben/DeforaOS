SUBDIRS	= sql
TARGETS	= daportal.1 daportal.html daportal.conf.5 daportal.conf.html install.html internals.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

daportal.1: daportal.xml
	./docbook.sh -P "$(PREFIX)" -- "daportal.1"

daportal.html: daportal.xml
	./docbook.sh -P "$(PREFIX)" -- "daportal.html"

daportal.conf.5: daportal.conf.xml
	./docbook.sh -P "$(PREFIX)" -- "daportal.conf.5"

daportal.conf.html: daportal.conf.xml
	./docbook.sh -P "$(PREFIX)" -- "daportal.conf.html"

install.html: install.xml install.xsl
	./docbook.sh -P "$(PREFIX)" -- "install.html"

internals.html: internals.xml internals.xsl
	./docbook.sh -P "$(PREFIX)" -- "internals.html"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(daportal.1_OBJS) $(daportal.html_OBJS) $(daportal.conf.5_OBJS) $(daportal.conf.html_OBJS) $(install.html_OBJS) $(internals.html_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(daportal.1_OBJS) $(daportal.html_OBJS) $(daportal.conf.5_OBJS) $(daportal.conf.html_OBJS) $(install.html_OBJS) $(internals.html_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "daportal.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "daportal.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "daportal.conf.5"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "daportal.conf.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "install.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "internals.html"

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "daportal.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "daportal.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "daportal.conf.5"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "daportal.conf.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "install.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "internals.html"

.PHONY: all subdirs clean distclean install uninstall
