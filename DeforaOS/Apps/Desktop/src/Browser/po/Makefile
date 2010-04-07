TARGETS	= Browser.pot de.po de.mo es.po es.mo fr.po fr.mo it.po it.mo
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

Browser.pot: POTFILES
	./gettext.sh "Browser.pot"

de.po: Browser.pot
	./gettext.sh "de.po"

de.mo: de.po
	./gettext.sh "de.mo"

es.po: Browser.pot
	./gettext.sh "es.po"

es.mo: es.po
	./gettext.sh "es.mo"

fr.po: Browser.pot
	./gettext.sh "fr.po"

fr.mo: fr.po
	./gettext.sh "fr.mo"

it.po: Browser.pot
	./gettext.sh "it.po"

it.mo: it.po
	./gettext.sh "it.mo"

clean:
	$(RM) $(Browser.pot_OBJS) $(de.po_OBJS) $(de.mo_OBJS) $(es.po_OBJS) $(es.mo_OBJS) $(fr.po_OBJS) $(fr.mo_OBJS) $(it.po_OBJS) $(it.mo_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	./gettext.sh -p "$(PREFIX)" install "de.mo"
	./gettext.sh -p "$(PREFIX)" install "es.mo"
	./gettext.sh -p "$(PREFIX)" install "fr.mo"
	./gettext.sh -p "$(PREFIX)" install "it.mo"

uninstall:
	./gettext.sh -p "$(PREFIX)" uninstall "de.mo"
	./gettext.sh -p "$(PREFIX)" uninstall "es.mo"
	./gettext.sh -p "$(PREFIX)" uninstall "fr.mo"
	./gettext.sh -p "$(PREFIX)" uninstall "it.mo"

.PHONY: all clean distclean install uninstall
