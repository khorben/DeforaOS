TARGETS	= mailing-lists.so search.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../../include
CPPFLAGS?=
CFLAGSF	= -W -fPIC `pkg-config --cflags libDesktop`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs openssl` -lssl `pkg-config --libs libDesktop`
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

mailing-lists_OBJS = mailing-lists.o
mailing-lists_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
mailing-lists_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L.. -Wl,-rpath,$(PREFIX)/lib -lMailer

mailing-lists.so: $(mailing-lists_OBJS)
	$(CCSHARED) -o mailing-lists.so $(mailing-lists_OBJS) $(mailing-lists_LDFLAGS)

search_OBJS = search.o
search_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
search_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

search.so: $(search_OBJS)
	$(CCSHARED) -o search.so $(search_OBJS) $(search_LDFLAGS)

mailing-lists.o: mailing-lists.c ../../include/Mailer.h
	$(CC) $(mailing-lists_CFLAGS) -c mailing-lists.c

search.o: search.c ../../include/Mailer.h
	$(CC) $(search_CFLAGS) -c search.c

clean:
	$(RM) -- $(mailing-lists_OBJS) $(search_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/plugins
	$(INSTALL) -m 0644 -- mailing-lists.so $(DESTDIR)$(LIBDIR)/Mailer/plugins/mailing-lists.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/plugins
	$(INSTALL) -m 0644 -- search.so $(DESTDIR)$(LIBDIR)/Mailer/plugins/search.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Mailer/plugins/mailing-lists.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Mailer/plugins/search.so

.PHONY: all clean distclean install uninstall
