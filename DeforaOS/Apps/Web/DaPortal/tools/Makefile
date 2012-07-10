TARGETS	= daportal
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

daportal: daportal.in
	./subst.sh -P "$(PREFIX)" -- "daportal"

clean:
	$(RM) -- $(daportal_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./subst.sh -P "$(DESTDIR)$(PREFIX)" -i -- "daportal"

uninstall:
	./subst.sh -P "$(DESTDIR)$(PREFIX)" -u -- "daportal"

.PHONY: all clean distclean install uninstall
