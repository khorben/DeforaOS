TARGETS	= ../src
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

../src: phplint.sh
	./phplint.sh -P "$(PREFIX)" -- "../src"

clean:
	$(RM) -- $(../src_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
