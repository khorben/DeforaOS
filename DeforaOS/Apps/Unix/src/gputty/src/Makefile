TARGETS	= gputty
CFLAGSF	= -Wall -ansi `pkg-config --cflags gtk+-2.0`
CFLAGS	= -W -g
LDFLAGSF= `pkg-config --libs gtk+-2.0`
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

GPUTTY_OBJS = config.o gputty.o
gputty: $(GPUTTY_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o gputty $(GPUTTY_OBJS)

config.o: config.c config.h
	$(CC) $(CFLAGSF) $(CFLAGS) -c config.c

gputty.o: gputty.c gputty.h
	$(CC) $(CFLAGSF) $(CFLAGS) -c gputty.c

gputty.h: config.h

clean:
	$(RM) $(GPUTTY_OBJS)

distclean: clean
	$(RM) $(TARGETS)
