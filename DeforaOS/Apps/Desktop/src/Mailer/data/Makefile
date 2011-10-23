SUBDIRS	= 16x16 24x24 48x48
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f
TARGETS	= Mailer.pc
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

Mailer.pc: Mailer.pc.in ../config.sh
	./pkgconfig.sh -P "$(PREFIX)" -- "Mailer.pc"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(Mailer.pc_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(Mailer.pc_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -i -- "Mailer.pc"
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- mailer.desktop $(DESTDIR)$(PREFIX)/share/applications/mailer.desktop

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -u -- "Mailer.pc"
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/mailer.desktop

.PHONY: all subdirs clean distclean install uninstall
