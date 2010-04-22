TARGETS	= Panel.pot fr.mo
PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

Panel.pot: POTFILES
	./gettext.sh "Panel.pot"

fr.mo: Panel.pot fr.po
	./gettext.sh "fr.mo"

clean:
	$(RM) $(Panel.pot_OBJS) $(fr.mo_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	./gettext.sh -p "$(PREFIX)" install "fr.mo"

uninstall:
	./gettext.sh -p "$(PREFIX)" uninstall "fr.mo"

.PHONY: all clean distclean install uninstall
