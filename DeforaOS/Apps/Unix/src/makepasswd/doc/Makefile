TARGETS	= makepasswd.1
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

makepasswd.1: makepasswd.1.xml
	./db2man.sh -P "$(PREFIX)" -- "makepasswd.1"

clean:
	$(RM) -- $(makepasswd.1_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./db2man.sh -P "$(DESTDIR)$(PREFIX)" -i -- "makepasswd.1"

uninstall:
	./db2man.sh -P "$(DESTDIR)$(PREFIX)" -u -- "makepasswd.1"

.PHONY: all clean distclean install uninstall
