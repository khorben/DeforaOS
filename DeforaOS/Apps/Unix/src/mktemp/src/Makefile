TARGETS	= mktemp
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

mktemp_OBJS= mktemp.o
mktemp_CFLAGS=$(CFLAGSF) $(CFLAGS)
mktemp: $(mktemp_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o mktemp $(mktemp_OBJS)

mktemp.o: mktemp.c
	$(CC) $(mktemp_CFLAGS) -c mktemp.c


clean:
	$(RM) $(mktemp_OBJS)

distclean: clean
	$(RM) $(TARGETS)
