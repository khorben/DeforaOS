TARGETS	= sh
CC	= cc
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
RM	= rm -f


all: $(TARGETS)

sh_OBJS = token.o scanner.o parser.o builtin.o job.o sh.o
sh_CFLAGS = $(CFLAGSF) $(CFLAGS)
sh: $(sh_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o sh $(sh_OBJS)

token.o: token.c
	$(CC) $(sh_CFLAGS) -c token.c

scanner.o: scanner.c
	$(CC) $(sh_CFLAGS) -c scanner.c

parser.o: parser.c
	$(CC) $(sh_CFLAGS) -c parser.c

builtin.o: builtin.c
	$(CC) $(sh_CFLAGS) -c builtin.c

job.o: job.c
	$(CC) $(sh_CFLAGS) -c job.c

sh.o: sh.c
	$(CC) $(sh_CFLAGS) -c sh.c

clean:
	$(RM) $(sh_OBJS)

distclean: clean
	$(RM) $(TARGETS)
