TARGETS	= as.pc
PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

as.pc: as.pc.in ../config.sh
	./pkgconfig.sh -P "$(PREFIX)" -- "as.pc"

clean:
	$(RM) -- $(as.pc_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -i -- "as.pc"

uninstall:
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -u -- "as.pc"

.PHONY: all clean distclean install uninstall
