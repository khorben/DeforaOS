SUBDIRS	= scripts
TARGETS	= configure.html project.conf.5 project.conf.html
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

project.conf.5: project.conf.xml
	./docbook.sh -P "$(PREFIX)" -- "project.conf.5"

project.conf.html: project.conf.xml
	./docbook.sh -P "$(PREFIX)" -- "project.conf.html"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(configure.html_OBJS) $(project.conf.5_OBJS) $(project.conf.html_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(configure.html_OBJS) $(project.conf.5_OBJS) $(project.conf.html_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "configure.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "project.conf.5"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "project.conf.html"

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "configure.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "project.conf.5"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "project.conf.html"

.PHONY: all subdirs clean distclean install uninstall
