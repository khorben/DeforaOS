TARGETS	= sh
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g -D_GNU_SOURCE -DDEBUG
LDFLAGSF=
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

sh: cmd.o parser.o prefs.o sh.o token.o tokencode.o tokenlist.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o $@ cmd.o parser.o prefs.o sh.o token.o tokencode.o tokenlist.o


cmd.o:
	$(CC) $(CFLAGSF) $(CFLAGS) -c cmd.c

parser.o:
	$(CC) $(CFLAGSF) $(CFLAGS) -c parser.c

prefs.o:
	$(CC) $(CFLAGSF) $(CFLAGS) -c prefs.c

sh.o:
	$(CC) $(CFLAGSF) $(CFLAGS) -c sh.c

token.o:
	$(CC) $(CFLAGSF) $(CFLAGS) -c token.c

tokencode.o:
	$(CC) $(CFLAGSF) $(CFLAGS) -c tokencode.c

tokenlist.o:
	$(CC) $(CFLAGSF) $(CFLAGS) -c tokenlist.c


clean:
	$(RM) cmd.o parser.o prefs.o sh.o token.o tokencode.o tokenlist.o

distclean: clean
	$(RM) $(TARGETS)
