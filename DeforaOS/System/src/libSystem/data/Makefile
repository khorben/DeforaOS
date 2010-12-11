TARGETS	= libSystem.pc
PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libSystem.pc: libSystem.pc.in ../config.sh
	./pkgconfig.sh "libSystem.pc"

clean:
	$(RM) -- $(libSystem.pc_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	./pkgconfig.sh -p "$(DESTDIR)$(PREFIX)" install "libSystem.pc"

uninstall:
	./pkgconfig.sh -p "$(DESTDIR)$(PREFIX)" uninstall "libSystem.pc"

.PHONY: all clean distclean install uninstall
