TARGETS	= configure
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF= -lSystem
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

configure_OBJS= configure.o
configure_CFLAGS=$(CFLAGSF) $(CFLAGS)
configure: $(configure_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o configure $(configure_OBJS)

configure.o: configure.c
	$(CC) $(configure_CFLAGS) -c configure.c


clean:
	$(RM) $(configure_OBJS)

distclean: clean
	$(RM) $(TARGETS)
