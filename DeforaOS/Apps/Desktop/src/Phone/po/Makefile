TARGETS	= Phone.pot fr.mo
PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

Phone.pot: POTFILES
	./gettext.sh "Phone.pot"

fr.mo: Phone.pot fr.po
	./gettext.sh "fr.mo"

clean:
	$(RM) $(Phone.pot_OBJS) $(fr.mo_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	./gettext.sh -p "$(DESTDIR)$(PREFIX)" install "fr.mo"

uninstall:
	./gettext.sh -p "$(DESTDIR)$(PREFIX)" uninstall "fr.mo"

.PHONY: all clean distclean install uninstall
