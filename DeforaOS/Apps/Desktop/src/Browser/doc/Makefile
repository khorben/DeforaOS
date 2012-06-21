TARGETS	= browser.1 browser.html desktop.1 desktop.html desktopctl.1 desktopctl.html open.1 open.html properties.1 properties.html
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

desktop.1: desktop.xml
	./docbook.sh -P "$(PREFIX)" -- "desktop.1"

desktop.html: desktop.xml
	./docbook.sh -P "$(PREFIX)" -- "desktop.html"

desktopctl.1: desktopctl.xml
	./docbook.sh -P "$(PREFIX)" -- "desktopctl.1"

desktopctl.html: desktopctl.xml
	./docbook.sh -P "$(PREFIX)" -- "desktopctl.html"

open.1: open.xml
	./docbook.sh -P "$(PREFIX)" -- "open.1"

open.html: open.xml
	./docbook.sh -P "$(PREFIX)" -- "open.html"

properties.1: properties.xml
	./docbook.sh -P "$(PREFIX)" -- "properties.1"

properties.html: properties.xml
	./docbook.sh -P "$(PREFIX)" -- "properties.html"

clean:
	$(RM) -- $(browser.1_OBJS) $(browser.html_OBJS) $(desktop.1_OBJS) $(desktop.html_OBJS) $(desktopctl.1_OBJS) $(desktopctl.html_OBJS) $(open.1_OBJS) $(open.html_OBJS) $(properties.1_OBJS) $(properties.html_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "browser.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "browser.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "desktop.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "desktop.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "desktopctl.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "desktopctl.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "open.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "open.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "properties.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "properties.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "browser.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "browser.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "desktop.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "desktop.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "desktopctl.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "desktopctl.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "open.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "open.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "properties.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "properties.html"

.PHONY: all clean distclean install uninstall
