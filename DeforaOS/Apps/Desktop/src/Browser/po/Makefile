TARGETS	= Browser.pot de.mo es.mo fr.mo
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

Browser.pot: POTFILES
	./gettext.sh "Browser.pot"

de.mo: Browser.pot de.po
	./gettext.sh "de.mo"

es.mo: Browser.pot es.po
	./gettext.sh "es.mo"

fr.mo: Browser.pot fr.po
	./gettext.sh "fr.mo"

clean:
	$(RM) $(Browser.pot_OBJS) $(de.mo_OBJS) $(es.mo_OBJS) $(fr.mo_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	./gettext.sh -p "$(PREFIX)" install "de.mo"
	./gettext.sh -p "$(PREFIX)" install "es.mo"
	./gettext.sh -p "$(PREFIX)" install "fr.mo"

uninstall:
	./gettext.sh -p "$(PREFIX)" uninstall "de.mo"
	./gettext.sh -p "$(PREFIX)" uninstall "es.mo"
	./gettext.sh -p "$(PREFIX)" uninstall "fr.mo"

.PHONY: all clean distclean install uninstall
