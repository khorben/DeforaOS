TARGETS	= surfer.1 surfer.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

surfer.1: surfer.xml
	./docbook.sh -P "$(PREFIX)" -- "surfer.1"

surfer.html: surfer.xml
	./docbook.sh -P "$(PREFIX)" -- "surfer.html"

clean:
	$(RM) -- $(surfer.1_OBJS) $(surfer.html_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "surfer.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "surfer.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "surfer.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "surfer.html"

.PHONY: all clean distclean install uninstall
