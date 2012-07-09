SUBDIRS	= sql
TARGETS	= daportal.conf.5 daportal.conf.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

daportal.conf.5: daportal.conf.xml
	./docbook.sh -P "$(PREFIX)" -- "daportal.conf.5"

daportal.conf.html: daportal.conf.xml
	./docbook.sh -P "$(PREFIX)" -- "daportal.conf.html"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(daportal.conf.5_OBJS) $(daportal.conf.html_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(daportal.conf.5_OBJS) $(daportal.conf.html_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "daportal.conf.5"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "daportal.conf.html"

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "daportal.conf.5"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "daportal.conf.html"

.PHONY: all subdirs clean distclean install uninstall
