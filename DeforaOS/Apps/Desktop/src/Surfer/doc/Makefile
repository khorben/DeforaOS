TARGETS	= download.1 download.html helper.1 helper.html surfer.1 surfer.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

download.1: download.xml
	./docbook.sh -P "$(PREFIX)" -- "download.1"

download.html: download.xml
	./docbook.sh -P "$(PREFIX)" -- "download.html"

helper.1: helper.xml
	./docbook.sh -P "$(PREFIX)" -- "helper.1"

helper.html: helper.xml
	./docbook.sh -P "$(PREFIX)" -- "helper.html"

surfer.1: surfer.xml
	./docbook.sh -P "$(PREFIX)" -- "surfer.1"

surfer.html: surfer.xml
	./docbook.sh -P "$(PREFIX)" -- "surfer.html"

clean:
	$(RM) -- $(download.1_OBJS) $(download.html_OBJS) $(helper.1_OBJS) $(helper.html_OBJS) $(surfer.1_OBJS) $(surfer.html_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "download.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "download.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "helper.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "helper.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "surfer.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "surfer.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "download.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "download.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "helper.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "helper.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "surfer.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "surfer.html"

.PHONY: all clean distclean install uninstall
