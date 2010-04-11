TARGETS	= Surfer.pot de.mo es.mo fr.mo it.mo
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

Surfer.pot: POTFILES
	./gettext.sh "Surfer.pot"

de.mo: Surfer.pot de.po
	./gettext.sh "de.mo"

es.mo: Surfer.pot es.po
	./gettext.sh "es.mo"

fr.mo: Surfer.pot fr.po
	./gettext.sh "fr.mo"

it.mo: Surfer.pot it.po
	./gettext.sh "it.mo"

clean:
	$(RM) $(Surfer.pot_OBJS) $(de.mo_OBJS) $(es.mo_OBJS) $(fr.mo_OBJS) $(it.mo_OBJS)

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
