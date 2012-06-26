TARGETS	= panel.1 panel.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

panel.1: panel.xml
	./docbook.sh -P "$(PREFIX)" -- "panel.1"

panel.html: panel.xml
	./docbook.sh -P "$(PREFIX)" -- "panel.html"

clean:
	$(RM) -- $(panel.1_OBJS) $(panel.html_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panel.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panel.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panel.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panel.html"

.PHONY: all clean distclean install uninstall
