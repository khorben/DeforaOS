TARGETS	= inetd echo
CFLAGSF	= -W -Wall -ansi
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

inetd_OBJS= inetd.o
inetd_CFLAGS=$(CFLAGSF) $(CFLAGS)
inetd: $(inetd_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o inetd $(inetd_OBJS)

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
