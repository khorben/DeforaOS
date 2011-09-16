TARGETS	= libParser.pc
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

libParser.pc: libParser.pc.in ../config.sh
	./pkgconfig.sh -P "$(PREFIX)" -- "libParser.pc"

clean:
	$(RM) -- $(libParser.pc_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -i -- "libParser.pc"

uninstall:
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -u -- "libParser.pc"

.PHONY: all clean distclean install uninstall
