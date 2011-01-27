TARGETS	= libDesktop.pc
PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libDesktop.pc: libDesktop.pc.in ../config.sh
	./pkgconfig.sh -P "$(PREFIX)" -- "libDesktop.pc"

clean:
	$(RM) -- $(libDesktop.pc_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -i -- "libDesktop.pc"

uninstall:
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -u -- "libDesktop.pc"

.PHONY: all clean distclean install uninstall
