TARGETS	= inetd echo
CFLAGSF	= -W -Wall -ansi
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

inetd_OBJS= config.o parser.o scanner.o service.o token.o inetd.o
inetd_CFLAGS=$(CFLAGSF) $(CFLAGS)
inetd: $(inetd_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o inetd $(inetd_OBJS)

config.o: config.c
	$(CC) $(inetd_CFLAGS) -c config.c

parser.o: parser.c
	$(CC) $(inetd_CFLAGS) -c parser.c

scanner.o: scanner.c
	$(CC) $(inetd_CFLAGS) -c scanner.c

service.o: service.c
	$(CC) $(inetd_CFLAGS) -c service.c

token.o: token.c
	$(CC) $(inetd_CFLAGS) -c token.c

inetd.o: inetd.c
	$(CC) $(inetd_CFLAGS) -c inetd.c

echo_OBJS= echo.o
echo_CFLAGS=$(CFLAGSF) $(CFLAGS)
echo: $(echo_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o echo $(echo_OBJS)

echo.o: echo.c
	$(CC) $(echo_CFLAGS) -c echo.c


clean:
	$(RM) $(inetd_OBJS) $(echo_OBJS)

distclean: clean
	$(RM) $(TARGETS)
