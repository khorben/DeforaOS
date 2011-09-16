TARGETS	= mixer
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libDesktop`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libDesktop`
LDFLAGS	= 
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

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- mixer $(DESTDIR)$(BINDIR)/mixer

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/mixer

.PHONY: all clean distclean install uninstall
