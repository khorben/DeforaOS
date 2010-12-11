TARGETS	= cpp.pc
PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

cpp.pc: cpp.pc.in ../config.sh
	./pkgconfig.sh "cpp.pc"

clean:
	$(RM) -- $(cpp.pc_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	./pkgconfig.sh -p "$(DESTDIR)$(PREFIX)" install "cpp.pc"

uninstall:
	./pkgconfig.sh -p "$(DESTDIR)$(PREFIX)" uninstall "cpp.pc"

.PHONY: all clean distclean install uninstall
