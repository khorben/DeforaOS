TARGETS	= libDesktop.pc
PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libDesktop.pc: ../config.sh
	./pkgconfig.sh "libDesktop.pc"

clean:
	$(RM) -- $(libDesktop.pc_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	./pkgconfig.sh -p "$(DESTDIR)$(PREFIX)" install "libDesktop.pc"

uninstall:
	./pkgconfig.sh -p "$(DESTDIR)$(PREFIX)" uninstall "libDesktop.pc"

.PHONY: all clean distclean install uninstall
