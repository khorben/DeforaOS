TARGETS	= locker.1 locker.html lockerctl.1 lockerctl.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

locker.1: locker.xml
	./docbook.sh -P "$(PREFIX)" -- "locker.1"

locker.html: locker.xml
	./docbook.sh -P "$(PREFIX)" -- "locker.html"

lockerctl.1: lockerctl.xml
	./docbook.sh -P "$(PREFIX)" -- "lockerctl.1"

lockerctl.html: lockerctl.xml
	./docbook.sh -P "$(PREFIX)" -- "lockerctl.html"

clean:
	$(RM) -- $(locker.1_OBJS) $(locker.html_OBJS) $(lockerctl.1_OBJS) $(lockerctl.html_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "locker.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "locker.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "lockerctl.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "lockerctl.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "locker.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "locker.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "lockerctl.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "lockerctl.html"

.PHONY: all clean distclean install uninstall
