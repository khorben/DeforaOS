CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF= -lcrypt
LDFLAGS	=
TARGETS	= makepasswd
CC	= gcc
RM	= rm -f


all: $(TARGETS)

MAKEPASSWD_OBJS= md5.o makepasswd.o
makepasswd: $(MAKEPASSWD_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) $(MAKEPASSWD_OBJS) -o makepasswd

makepasswd.o: makepasswd.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c makepasswd.c

md5.o: md5.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c md5.c

clean:
	$(RM) $(MAKEPASSWD_OBJS)

distclean: clean
	$(RM) $(TARGETS)
