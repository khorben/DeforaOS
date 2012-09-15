TARGETS	= C99.pc
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

C99.pc: C99.pc.in ../config.sh
	./pkgconfig.sh -P "$(PREFIX)" -- "C99.pc"

clean:
	$(RM) -- $(C99.pc_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -i -- "C99.pc"

uninstall:
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -u -- "C99.pc"

.PHONY: all clean distclean install uninstall
