TARGETS	= configure
CC	= cc
CFLAGSF	= -W -Wall -ansi -I /System/Include
CFLAGS	= -g
LDFLAGSF= -L /System/Libraries -l System -l dl
RM	= rm -f


all: $(TARGETS)

configure_OBJS = configure.o
configure_CFLAGS = $(CFLAGSF) $(CFLAGS)
configure: $(configure_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o configure $(configure_OBJS)

configure.o: configure.c
	$(CC) $(configure_CFLAGS) -c configure.c

clean:
	$(RM) $(configure_OBJS)

distclean: clean
	$(RM) $(TARGETS)
