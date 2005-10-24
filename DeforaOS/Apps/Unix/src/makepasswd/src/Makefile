TARGETS	= makepasswd
CC	= cc
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGS	= -lcrypt
RM	= rm -f


all: $(TARGETS)

makepasswd_OBJS = makepasswd.o md5.o
makepasswd_CFLAGS = $(CFLAGSF) $(CFLAGS)
makepasswd: $(makepasswd_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o makepasswd $(makepasswd_OBJS)

makepasswd.o: makepasswd.c
	$(CC) $(makepasswd_CFLAGS) -c makepasswd.c

md5.o: md5.c
	$(CC) $(makepasswd_CFLAGS) -c md5.c

clean:
	$(RM) $(makepasswd_OBJS)

distclean: clean
	$(RM) $(TARGETS)
