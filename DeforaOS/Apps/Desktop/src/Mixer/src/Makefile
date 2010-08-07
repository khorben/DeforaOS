TARGETS	= mixer
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I $(PREFIX)/include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs gtk+-2.0` -lDesktop
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

mixer_OBJS = mixer.o callbacks.o main.o
mixer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
mixer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

mixer: $(mixer_OBJS)
	$(CC) -o mixer $(mixer_OBJS) $(mixer_LDFLAGS)

mixer.o: mixer.c callbacks.h mixer.h ../config.h
	$(CC) $(mixer_CFLAGS) -c mixer.c

callbacks.o: callbacks.c mixer.h callbacks.h ../config.h
	$(CC) $(mixer_CFLAGS) -c callbacks.c

main.o: main.c mixer.h ../config.h
	$(CC) $(mixer_CFLAGS) -c main.c

clean:
	$(RM) -- $(mixer_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- mixer $(DESTDIR)$(BINDIR)/mixer

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/mixer

.PHONY: all clean distclean install uninstall
