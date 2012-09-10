SUBDIRS	= gtkdoc
TARGETS	= compose.1 compose.html gtkdoc/Mailer.types gtkdoc/tmpl.stamp gtkdoc/sgml.stamp gtkdoc/html.stamp mailer.1 mailer.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

compose.1: compose.xml
	./docbook.sh -P "$(PREFIX)" -- "compose.1"

compose.html: compose.xml
	./docbook.sh -P "$(PREFIX)" -- "compose.html"

gtkdoc/Mailer.types: ../config.sh
	./gtkdoc.sh -P "$(PREFIX)" -- "gtkdoc/Mailer.types"

gtkdoc/tmpl.stamp: gtkdoc/Mailer.types
	./gtkdoc.sh -P "$(PREFIX)" -- "gtkdoc/tmpl.stamp"

gtkdoc/sgml.stamp: gtkdoc/tmpl.stamp
	./gtkdoc.sh -P "$(PREFIX)" -- "gtkdoc/sgml.stamp"

gtkdoc/html.stamp: gtkdoc/Mailer-docs.xml gtkdoc/sgml.stamp
	./gtkdoc.sh -P "$(PREFIX)" -- "gtkdoc/html.stamp"

mailer.1: mailer.xml
	./docbook.sh -P "$(PREFIX)" -- "mailer.1"

mailer.html: mailer.xml
	./docbook.sh -P "$(PREFIX)" -- "mailer.html"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(compose.1_OBJS) $(compose.html_OBJS) $(gtkdoc/Mailer.types_OBJS) $(gtkdoc/tmpl.stamp_OBJS) $(gtkdoc/sgml.stamp_OBJS) $(gtkdoc/html.stamp_OBJS) $(mailer.1_OBJS) $(mailer.html_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(compose.1_OBJS) $(compose.html_OBJS) $(gtkdoc/Mailer.types_OBJS) $(gtkdoc/tmpl.stamp_OBJS) $(gtkdoc/sgml.stamp_OBJS) $(gtkdoc/html.stamp_OBJS) $(mailer.1_OBJS) $(mailer.html_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "compose.html"
	./gtkdoc.sh -P "$(DESTDIR)$(PREFIX)" -i -- "gtkdoc/html.stamp"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "mailer.html"

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "compose.html"
	./gtkdoc.sh -P "$(DESTDIR)$(PREFIX)" -u -- "gtkdoc/html.stamp"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "mailer.html"

.PHONY: all subdirs clean distclean install uninstall
