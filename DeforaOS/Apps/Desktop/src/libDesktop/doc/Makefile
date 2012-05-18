SUBDIRS	= gtkdoc
TARGETS	= gtkdoc/libDesktop.types gtkdoc/tmpl.stamp gtkdoc/sgml.stamp gtkdoc/html.stamp
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

gtkdoc/libDesktop.types: ../config.sh
	./gtkdoc.sh -P "$(PREFIX)" -- "gtkdoc/libDesktop.types"

gtkdoc/tmpl.stamp: gtkdoc/libDesktop.types
	./gtkdoc.sh -P "$(PREFIX)" -- "gtkdoc/tmpl.stamp"

gtkdoc/sgml.stamp: gtkdoc/tmpl.stamp
	./gtkdoc.sh -P "$(PREFIX)" -- "gtkdoc/sgml.stamp"

gtkdoc/html.stamp: gtkdoc/libDesktop-docs.xml gtkdoc/sgml.stamp
	./gtkdoc.sh -P "$(PREFIX)" -- "gtkdoc/html.stamp"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(gtkdoc/libDesktop.types_OBJS) $(gtkdoc/tmpl.stamp_OBJS) $(gtkdoc/sgml.stamp_OBJS) $(gtkdoc/html.stamp_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(gtkdoc/libDesktop.types_OBJS) $(gtkdoc/tmpl.stamp_OBJS) $(gtkdoc/sgml.stamp_OBJS) $(gtkdoc/html.stamp_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	./gtkdoc.sh -P "$(DESTDIR)$(PREFIX)" -i -- "gtkdoc/html.stamp"

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	./gtkdoc.sh -P "$(DESTDIR)$(PREFIX)" -u -- "gtkdoc/html.stamp"

.PHONY: all subdirs clean distclean install uninstall
