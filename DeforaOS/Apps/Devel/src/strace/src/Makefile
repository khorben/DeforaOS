TARGETS	= strace
CFLAGS	= -W -Wall -ansi
CFLAGSF	= -g
LDFLAGSF=
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)


STRACE_OBJS = linux.o strace.o
strace: $(STRACE_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o strace $(STRACE_OBJS)


linux.o: linux.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c linux.c

strace.o: strace.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c strace.c


clean:
	$(RM) $(STRACE_OBJS)

distclean: clean
	$(RM) $(TARGETS)
