TARGETS	= as
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

as_OBJS= as.o parser.o scanner.o token.o
as_CFLAGS=$(CFLAGSF) $(CFLAGS)
as: $(as_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o as $(as_OBJS)

as.o: as.c
	$(CC) $(as_CFLAGS) -c as.c

parser.o: parser.c
	$(CC) $(as_CFLAGS) -c parser.c

scanner.o: scanner.c
	$(CC) $(as_CFLAGS) -c scanner.c

token.o: token.c
	$(CC) $(as_CFLAGS) -c token.c


clean:
	$(RM) $(as_OBJS)

distclean: clean
	$(RM) $(TARGETS)
