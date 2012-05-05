TARGETS	= configure.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

configure.html: configure.xml
	./docbook.sh -P "$(PREFIX)" -- "configure.html"

clean:
	$(RM) -- $(configure.html_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "configure.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "configure.html"

.PHONY: all clean distclean install uninstall
