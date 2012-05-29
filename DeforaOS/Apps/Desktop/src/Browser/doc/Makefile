TARGETS	= browser.1 browser.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

browser.1: browser.xml
	./docbook.sh -P "$(PREFIX)" -- "browser.1"

browser.html: browser.xml
	./docbook.sh -P "$(PREFIX)" -- "browser.html"

clean:
	$(RM) -- $(browser.1_OBJS) $(browser.html_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "browser.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "browser.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "browser.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "browser.html"

.PHONY: all clean distclean install uninstall
