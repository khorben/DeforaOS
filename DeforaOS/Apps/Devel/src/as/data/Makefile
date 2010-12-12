TARGETS	= as.pc
PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

as.pc: as.pc.in ../config.sh
	./pkgconfig.sh "as.pc"

clean:
	$(RM) -- $(as.pc_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	./pkgconfig.sh -p "$(DESTDIR)$(PREFIX)" install "as.pc"

uninstall:
	./pkgconfig.sh -p "$(DESTDIR)$(PREFIX)" uninstall "as.pc"

.PHONY: all clean distclean install uninstall
