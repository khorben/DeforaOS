TARGETS	= configure
CFLAGS	= -W -Wall -ansi
CFLAGSF	= -g
LDFLAGSF= -lutils
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

configure: configure.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o configure configure.o


configure.o: configure.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c configure.c


clean:
	$(RM) configure.o

distclean: clean
	$(RM) $(TARGETS)
