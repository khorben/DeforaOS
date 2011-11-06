TARGETS	= cvs.so dirtree.so preview.so properties.so subversion.so volumes.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libDesktop`
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

cvs_OBJS = cvs.o
cvs_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
cvs_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

cvs.so: $(cvs_OBJS)
	$(CCSHARED) -o cvs.so $(cvs_OBJS) $(cvs_LDFLAGS)

dirtree_OBJS = dirtree.o
dirtree_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
dirtree_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

dirtree.so: $(dirtree_OBJS)
	$(CCSHARED) -o dirtree.so $(dirtree_OBJS) $(dirtree_LDFLAGS)

preview_OBJS = preview.o
preview_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
preview_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

preview.so: $(preview_OBJS)
	$(CCSHARED) -o preview.so $(preview_OBJS) $(preview_LDFLAGS)

properties_OBJS = properties.o
properties_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
properties_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

properties.so: $(properties_OBJS)
	$(CCSHARED) -o properties.so $(properties_OBJS) $(properties_LDFLAGS)

subversion_OBJS = subversion.o
subversion_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
subversion_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

subversion.so: $(subversion_OBJS)
	$(CCSHARED) -o subversion.so $(subversion_OBJS) $(subversion_LDFLAGS)

volumes_OBJS = volumes.o
volumes_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
volumes_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

volumes.so: $(volumes_OBJS)
	$(CCSHARED) -o volumes.so $(volumes_OBJS) $(volumes_LDFLAGS)

cvs.o: cvs.c ../../include/Browser.h
	$(CC) $(cvs_CFLAGS) -c cvs.c

dirtree.o: dirtree.c ../../include/Browser.h
	$(CC) $(dirtree_CFLAGS) -c dirtree.c

preview.o: preview.c ../../include/Browser.h
	$(CC) $(preview_CFLAGS) -c preview.c

properties.o: properties.c ../../include/Browser.h
	$(CC) $(properties_CFLAGS) -c properties.c

subversion.o: subversion.c ../../include/Browser.h
	$(CC) $(subversion_CFLAGS) -c subversion.c

volumes.o: volumes.c ../../include/Browser.h
	$(CC) $(volumes_CFLAGS) -c volumes.c

clean:
	$(RM) -- $(cvs_OBJS) $(dirtree_OBJS) $(preview_OBJS) $(properties_OBJS) $(subversion_OBJS) $(volumes_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Browser/plugins
	$(INSTALL) -m 0644 -- cvs.so $(DESTDIR)$(LIBDIR)/Browser/plugins/cvs.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Browser/plugins
	$(INSTALL) -m 0644 -- dirtree.so $(DESTDIR)$(LIBDIR)/Browser/plugins/dirtree.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Browser/plugins
	$(INSTALL) -m 0644 -- preview.so $(DESTDIR)$(LIBDIR)/Browser/plugins/preview.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Browser/plugins
	$(INSTALL) -m 0644 -- properties.so $(DESTDIR)$(LIBDIR)/Browser/plugins/properties.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Browser/plugins
	$(INSTALL) -m 0644 -- subversion.so $(DESTDIR)$(LIBDIR)/Browser/plugins/subversion.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Browser/plugins
	$(INSTALL) -m 0644 -- volumes.so $(DESTDIR)$(LIBDIR)/Browser/plugins/volumes.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Browser/plugins/cvs.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Browser/plugins/dirtree.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Browser/plugins/preview.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Browser/plugins/properties.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Browser/plugins/subversion.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Browser/plugins/volumes.so

.PHONY: all clean distclean install uninstall
