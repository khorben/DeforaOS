TARGETS	= gputty
CFLAGS	= -W -Wall -ansi `pkg-config --cflags gtk+-2.0`
CFLAGGF	= -g
LDFLAGSF= `pkg-config --libs gtk+-2.0`
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

gputty: gputty.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o gputty gputty.o

gputty.o: gputty.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c gputty.c

clean:
	$(RM) gputty.o

distclean: clean
	$(RM) $(TARGETS)
