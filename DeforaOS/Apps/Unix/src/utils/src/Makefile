TARGETS	= false true
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF=
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

false: false.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o false false.o

true: true.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o true true.o


false.o: false.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c false.c

true.o: true.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c true.c


clean:
	$(RM) false.o true.o

distclean: clean
	$(RM) $(TARGETS)
