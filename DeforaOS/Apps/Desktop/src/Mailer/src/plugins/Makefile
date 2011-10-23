TARGETS	= search.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../../include
CPPFLAGS?=
CFLAGSF	= -W -fPIC `pkg-config --cflags glib-2.0`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs glib-2.0`
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

search_OBJS = search.o
search_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
search_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs gtk+-2.0`

search.so: $(search_OBJS)
	$(CCSHARED) -o search.so $(search_OBJS) $(search_LDFLAGS)

search.o: search.c ../../include/Mailer.h
	$(CC) $(search_CFLAGS) `pkg-config --cflags gtk+-2.0` -c search.c

clean:
	$(RM) -- $(search_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/plugins
	$(INSTALL) -m 0644 -- search.so $(DESTDIR)$(LIBDIR)/Mailer/plugins/search.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Mailer/plugins/search.so

.PHONY: all clean distclean install uninstall
