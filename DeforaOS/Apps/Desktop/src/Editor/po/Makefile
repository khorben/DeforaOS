TARGETS	= Editor.pot de.mo es.mo fr.mo it.mo
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

Editor.pot: POTFILES
	./gettext.sh -P "$(PREFIX)" -- "Editor.pot"

de.mo: Editor.pot de.po
	./gettext.sh -P "$(PREFIX)" -- "de.mo"

es.mo: Editor.pot es.po
	./gettext.sh -P "$(PREFIX)" -- "es.mo"

fr.mo: Editor.pot fr.po
	./gettext.sh -P "$(PREFIX)" -- "fr.mo"

it.mo: Editor.pot it.po
	./gettext.sh -P "$(PREFIX)" -- "it.mo"

clean:
	$(RM) -- $(Editor.pot_OBJS) $(de.mo_OBJS) $(es.mo_OBJS) $(fr.mo_OBJS) $(it.mo_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -i -- "de.mo"
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -i -- "es.mo"
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -i -- "fr.mo"
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -i -- "it.mo"

uninstall:
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -u -- "de.mo"
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -u -- "es.mo"
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -u -- "fr.mo"
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -u -- "it.mo"

.PHONY: all clean distclean install uninstall
