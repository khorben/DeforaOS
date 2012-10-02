PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/database
	$(INSTALL) -m 0644 -- pdo.php $(DESTDIR)$(PREFIX)/daportal/src/database/pdo.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/database
	$(INSTALL) -m 0644 -- pgsql.php $(DESTDIR)$(PREFIX)/daportal/src/database/pgsql.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/database
	$(INSTALL) -m 0644 -- sqlite2.php $(DESTDIR)$(PREFIX)/daportal/src/database/sqlite2.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/database
	$(INSTALL) -m 0644 -- sqlite3.php $(DESTDIR)$(PREFIX)/daportal/src/database/sqlite3.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/database/pdo.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/database/pgsql.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/database/sqlite2.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/database/sqlite3.php

.PHONY: all clean distclean install uninstall
