TARGETS	= DaPortal.pot fr.mo
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

DaPortal.pot: POTFILES
	./gettext.sh -P "$(PREFIX)" -- "DaPortal.pot"

fr.mo: DaPortal.pot fr.po
	./gettext.sh -P "$(PREFIX)" -- "fr.mo"

clean:
	$(RM) -- $(DaPortal.pot_OBJS) $(fr.mo_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -i -- "fr.mo"

uninstall:
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -u -- "fr.mo"

.PHONY: all clean distclean install uninstall
