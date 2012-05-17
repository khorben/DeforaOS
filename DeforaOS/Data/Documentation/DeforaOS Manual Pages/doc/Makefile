TARGETS	= manual/manual.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

manual/manual.html: manual/manual.xml manual/manual.xsl ../config.sh
	./docbook.sh -P "$(PREFIX)" -- "manual/manual.html"

clean:
	$(RM) -- $(manual/manual.html_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
