TARGETS	= panel.1 panel.html panelctl.1 panelctl.html panel-embed.1 panel-embed.html panel-message.1 panel-message.html panel-notify.1 panel-notify.html
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

panel.1: panel.xml
	./docbook.sh -P "$(PREFIX)" -- "panel.1"

panel.html: panel.xml
	./docbook.sh -P "$(PREFIX)" -- "panel.html"

panelctl.1: panelctl.xml
	./docbook.sh -P "$(PREFIX)" -- "panelctl.1"

panelctl.html: panelctl.xml
	./docbook.sh -P "$(PREFIX)" -- "panelctl.html"

panel-embed.1: panel-embed.xml
	./docbook.sh -P "$(PREFIX)" -- "panel-embed.1"

panel-embed.html: panel-embed.xml
	./docbook.sh -P "$(PREFIX)" -- "panel-embed.html"

panel-message.1: panel-message.xml
	./docbook.sh -P "$(PREFIX)" -- "panel-message.1"

panel-message.html: panel-message.xml
	./docbook.sh -P "$(PREFIX)" -- "panel-message.html"

panel-notify.1: panel-notify.xml
	./docbook.sh -P "$(PREFIX)" -- "panel-notify.1"

panel-notify.html: panel-notify.xml
	./docbook.sh -P "$(PREFIX)" -- "panel-notify.html"

clean:
	$(RM) -- $(panel.1_OBJS) $(panel.html_OBJS) $(panelctl.1_OBJS) $(panelctl.html_OBJS) $(panel-embed.1_OBJS) $(panel-embed.html_OBJS) $(panel-message.1_OBJS) $(panel-message.html_OBJS) $(panel-notify.1_OBJS) $(panel-notify.html_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panel.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panel.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panelctl.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panelctl.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panel-embed.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panel-embed.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panel-message.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panel-message.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panel-notify.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "panel-notify.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panel.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panel.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panelctl.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panelctl.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panel-embed.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panel-embed.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panel-message.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panel-message.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panel-notify.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "panel-notify.html"

.PHONY: all clean distclean install uninstall
