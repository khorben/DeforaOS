TARGETS	= sh
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g -D_GNU_SOURCE -DDEBUG
LDFLAGSF= -lutils
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

sh: cmd.o parser.o prefs.o sh.o token.o tokencode.o tokenlist.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o $@ cmd.o parser.o prefs.o sh.o token.o tokencode.o tokenlist.o


cmd.o: cmd.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c cmd.c

parser.o: parser.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c parser.c

prefs.o: prefs.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c prefs.c

sh.o: sh.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c sh.c

token.o: token.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c token.c

tokencode.o: tokencode.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c tokencode.c

tokenlist.o: tokenlist.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c tokenlist.c


clean:
	$(RM) cmd.o parser.o prefs.o sh.o token.o tokencode.o tokenlist.o

distclean: clean
	$(RM) $(TARGETS)
