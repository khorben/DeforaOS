TARGETS	= makepasswd.1 makepasswd.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

makepasswd.1: makepasswd.xml
	./docbook.sh -P "$(PREFIX)" -- "makepasswd.1"

makepasswd.html: makepasswd.xml
	./docbook.sh -P "$(PREFIX)" -- "makepasswd.html"

clean:
	$(RM) -- $(makepasswd.1_OBJS) $(makepasswd.html_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "makepasswd.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "makepasswd.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "makepasswd.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "makepasswd.html"

.PHONY: all clean distclean install uninstall
