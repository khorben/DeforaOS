SUBDIRS	= scripts
TARGETS	= configure.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

configure.html: configure.xml
	./docbook.sh -P "$(PREFIX)" -- "configure.html"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(configure.html_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(configure.html_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "configure.html"

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "configure.html"

.PHONY: all subdirs clean distclean install uninstall
