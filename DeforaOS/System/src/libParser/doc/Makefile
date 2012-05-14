SUBDIRS	= gtkdoc
TARGETS	= gtkdoc/libParser.types gtkdoc/sgml.stamp html/index.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

gtkdoc/libParser.types:
	./gtkdoc.sh -P "$(PREFIX)" -- "gtkdoc/libParser.types"

gtkdoc/sgml.stamp: gtkdoc/libParser.types
	./gtkdoc.sh -P "$(PREFIX)" -- "gtkdoc/sgml.stamp"

html/index.html: gtkdoc/libParser-docs.xml gtkdoc/sgml.stamp
	./gtkdoc.sh -P "$(PREFIX)" -- "html/index.html"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(gtkdoc/libParser.types_OBJS) $(gtkdoc/sgml.stamp_OBJS) $(html/index.html_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(gtkdoc/libParser.types_OBJS) $(gtkdoc/sgml.stamp_OBJS) $(html/index.html_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean install uninstall
