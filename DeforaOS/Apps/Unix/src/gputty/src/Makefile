TARGETS	= gputty
CFLAGS	= -W -Wall -ansi `pkg-config --cflags gtk+-2.0`
CFLAGGF	= -g
LDFLAGSF= `pkg-config --libs gtk+-2.0`
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

gputty: config.o gputty.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o gputty config.o gputty.o

config.o: config.c config.h
	$(CC) $(CFLAGSF) $(CFLAGS) -c config.c

gputty.o: gputty.c gputty.h
	$(CC) $(CFLAGSF) $(CFLAGS) -c gputty.c

gputty.h: config.h

clean:
	$(RM) gputty.o

distclean: clean
	$(RM) $(TARGETS)
