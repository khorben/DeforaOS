TARGETS	= Editor.pot de.po de.mo es.po es.mo fr.po fr.mo
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

Editor.pot: POTFILES
	./gettext.sh "Editor.pot"

de.po: Editor.pot
	./gettext.sh "de.po"

de.mo: de.po
	./gettext.sh "de.mo"

es.po: Editor.pot
	./gettext.sh "es.po"

es.mo: es.po
	./gettext.sh "es.mo"

fr.po: Editor.pot
	./gettext.sh "fr.po"

fr.mo: fr.po
	./gettext.sh "fr.mo"

clean:
	$(RM) $(Editor.pot_OBJS) $(de.po_OBJS) $(de.mo_OBJS) $(es.po_OBJS) $(es.mo_OBJS) $(fr.po_OBJS) $(fr.mo_OBJS)

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
