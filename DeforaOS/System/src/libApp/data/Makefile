TARGETS	= libApp.pc
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libApp.pc: libApp.pc.in ../config.sh
	./pkgconfig.sh -P "$(PREFIX)" -- "libApp.pc"

clean:
	$(RM) -- $(libApp.pc_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -i -- "libApp.pc"

uninstall:
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -u -- "libApp.pc"

.PHONY: all clean distclean install uninstall
