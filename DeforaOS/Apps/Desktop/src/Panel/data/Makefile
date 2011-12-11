SUBDIRS	= 16x16 22x22 24x24 32x32 48x48 scalable
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f
TARGETS	= Panel.pc
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

Panel.pc: Panel.pc.in ../config.sh
	./pkgconfig.sh -P "$(PREFIX)" -- "Panel.pc"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(Panel.pc_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(Panel.pc_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -i -- "Panel.pc"
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- panel-settings.desktop $(DESTDIR)$(PREFIX)/share/applications/panel-settings.desktop

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -u -- "Panel.pc"
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/panel-settings.desktop

.PHONY: all subdirs clean distclean install uninstall
