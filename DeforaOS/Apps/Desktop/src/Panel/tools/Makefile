TARGETS	= panel-embed panel-message panel-notify panel-test
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libSystem libDesktop`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config --libs libSystem libDesktop`
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

panel-embed_OBJS = embed.o
panel-embed_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
panel-embed_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

panel-embed: $(panel-embed_OBJS)
	$(CC) -o panel-embed $(panel-embed_OBJS) $(panel-embed_LDFLAGS)

panel-message_OBJS = message.o
panel-message_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
panel-message_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

panel-message: $(panel-message_OBJS)
	$(CC) -o panel-message $(panel-message_OBJS) $(panel-message_LDFLAGS)

panel-notify_OBJS = notify.o
panel-notify_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D PREFIX=\"$(PREFIX)\" $(CFLAGSF) $(CFLAGS)
panel-notify_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

panel-notify: $(panel-notify_OBJS)
	$(CC) -o panel-notify $(panel-notify_OBJS) $(panel-notify_LDFLAGS)

panel-test_OBJS = test.o
panel-test_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D PREFIX=\"$(PREFIX)\" $(CFLAGSF) $(CFLAGS)
panel-test_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

panel-test: $(panel-test_OBJS)
	$(CC) -o panel-test $(panel-test_OBJS) $(panel-test_LDFLAGS)

embed.o: embed.c ../include/Panel.h ../config.h
	$(CC) $(panel-embed_CFLAGS) -c embed.c

message.o: message.c ../include/Panel.h
	$(CC) $(panel-message_CFLAGS) -c message.c

notify.o: notify.c helper.c ../src/panel.h ../config.h
	$(CC) $(panel-notify_CFLAGS) -c notify.c

test.o: test.c helper.c ../src/panel.h ../config.h
	$(CC) $(panel-test_CFLAGS) -c test.c

clean:
	$(RM) -- $(panel-embed_OBJS) $(panel-message_OBJS) $(panel-notify_OBJS) $(panel-test_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- panel-embed $(DESTDIR)$(BINDIR)/panel-embed
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- panel-message $(DESTDIR)$(BINDIR)/panel-message
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- panel-notify $(DESTDIR)$(BINDIR)/panel-notify

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/panel-embed
	$(RM) -- $(DESTDIR)$(BINDIR)/panel-message
	$(RM) -- $(DESTDIR)$(BINDIR)/panel-notify

.PHONY: all clean distclean install uninstall
