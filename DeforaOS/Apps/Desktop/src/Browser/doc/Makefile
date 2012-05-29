TARGETS	= browser.1
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

browser.1: browser.xml
	./docbook.sh -P "$(PREFIX)" -- "browser.1"

clean:
	$(RM) -- $(browser.1_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "browser.1"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "browser.1"

.PHONY: all clean distclean install uninstall
