TARGETS	= mktemp
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF=
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

mktemp: mktemp.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o mktemp mktemp.o

mktemp.o: mktemp.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c mktemp.c

clean:
	$(RM) mktemp.o

distclean: clean
	$(RM) $(TARGETS)
