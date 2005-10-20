TARGETS	= libSystem.a libSystem.so
CFLAGSF	= -W -Wall -ansi -I ../include
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

libSystem_OBJS= appclient.o appinterface.o appserver.o array.o config.o event.o hash.o string.o
libSystem_CFLAGS=$(CFLAGSF) $(CFLAGS)
libSystem.a: $(libSystem_OBJS)
	$(AR) libSystem.a $(libSystem_OBJS)
	$(RANLIB) libSystem.a

libSystem.so: $(libSystem_OBJS)
	$(LD) -o libSystem.so $(libSystem_OBJS)

appclient.o: appclient.c
	$(CC) $(libSystem_CFLAGS) -c appclient.c

appinterface.o: appinterface.c
	$(CC) $(libSystem_CFLAGS) -c appinterface.c

appserver.o: appserver.c
	$(CC) $(libSystem_CFLAGS) -c appserver.c

array.o: array.c
	$(CC) $(libSystem_CFLAGS) -c array.c

config.o: config.c
	$(CC) $(libSystem_CFLAGS) -c config.c

event.o: event.c
	$(CC) $(libSystem_CFLAGS) -c event.c

hash.o: hash.c
	$(CC) $(libSystem_CFLAGS) -c hash.c

string.o: string.c
	$(CC) $(libSystem_CFLAGS) -c string.c


clean:
	$(RM) $(libSystem_OBJS)

distclean: clean
	$(RM) $(TARGETS)
