TARGETS	= Locker.pot fr.mo
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

Locker.pot: POTFILES
	./gettext.sh -P "$(PREFIX)" -- "Locker.pot"

fr.mo: Locker.pot fr.po
	./gettext.sh -P "$(PREFIX)" -- "fr.mo"

clean:
	$(RM) -- $(Locker.pot_OBJS) $(fr.mo_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -i -- "fr.mo"

uninstall:
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -u -- "fr.mo"

.PHONY: all clean distclean install uninstall
