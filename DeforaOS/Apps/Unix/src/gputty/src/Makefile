TARGETS	= gputty
CFLAGSF	= -Wall -ansi `pkg-config --cflags gtk+-2.0`
CFLAGS	= -W -g
LDFLAGSF= `pkg-config --libs gtk+-2.0`
LDFLAGS	=
CC	= cc
RM	= rm -f
INSTALL	= install


all: $(TARGETS)

GPUTTY_OBJS = array.o config.o gputty.o hash.o
gputty: $(GPUTTY_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o gputty $(GPUTTY_OBJS)

array.o: array.c array.h
	$(CC) $(CFLAGSF) $(CFLAGS) -c array.c

config.o: config.c config.h
	$(CC) $(CFLAGSF) $(CFLAGS) -c config.c

gputty.o: gputty.c gputty.h
	$(CC) $(CFLAGSF) $(CFLAGS) -c gputty.c

gputty.h: config.h

hash.o: hash.c hash.h array.h
	$(CC) $(CFLAGSF) $(CFLAGS) -c hash.c

clean:
	$(RM) $(GPUTTY_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(INSTALL) -m 755 gputty $(DESTDIR)$(BINDIR)
