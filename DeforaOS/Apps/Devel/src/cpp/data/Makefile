TARGETS	= cpp.pc
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

cpp.pc: cpp.pc.in ../config.sh
	./pkgconfig.sh -P "$(PREFIX)" -- "cpp.pc"

clean:
	$(RM) -- $(cpp.pc_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -i -- "cpp.pc"

uninstall:
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -u -- "cpp.pc"

.PHONY: all clean distclean install uninstall
