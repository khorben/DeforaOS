TARGETS	= libParser.pc
PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libParser.pc: ../config.sh
	./pkgconfig.sh "libParser.pc"

clean:
	$(RM) -- $(libParser.pc_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	./pkgconfig.sh -p "$(DESTDIR)$(PREFIX)" install "libParser.pc"

uninstall:
	./pkgconfig.sh -p "$(DESTDIR)$(PREFIX)" uninstall "libParser.pc"

.PHONY: all clean distclean install uninstall
