SUBDIRS	= locale
PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

install:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- auth.php $(DESTDIR)$(PREFIX)/daportal/src/system/auth.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- common.php $(DESTDIR)$(PREFIX)/daportal/src/system/common.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- config.php $(DESTDIR)$(PREFIX)/daportal/src/system/config.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- content.php $(DESTDIR)$(PREFIX)/daportal/src/system/content.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- database.php $(DESTDIR)$(PREFIX)/daportal/src/system/database.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- engine.php $(DESTDIR)$(PREFIX)/daportal/src/system/engine.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- html.php $(DESTDIR)$(PREFIX)/daportal/src/system/html.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- locale.php $(DESTDIR)$(PREFIX)/daportal/src/system/locale.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- mail.php $(DESTDIR)$(PREFIX)/daportal/src/system/mail.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- mime.php $(DESTDIR)$(PREFIX)/daportal/src/system/mime.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- module.php $(DESTDIR)$(PREFIX)/daportal/src/system/module.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- page.php $(DESTDIR)$(PREFIX)/daportal/src/system/page.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- request.php $(DESTDIR)$(PREFIX)/daportal/src/system/request.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- template.php $(DESTDIR)$(PREFIX)/daportal/src/system/template.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system
	$(INSTALL) -m 0644 -- user.php $(DESTDIR)$(PREFIX)/daportal/src/system/user.php

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/auth.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/common.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/config.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/content.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/database.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/engine.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/html.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/locale.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/mail.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/mime.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/module.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/page.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/request.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/template.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/user.php

.PHONY: all subdirs clean distclean install uninstall
