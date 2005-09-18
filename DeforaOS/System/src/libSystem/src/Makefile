TARGETS	= libSystem.a libSystem.so
CFLAGSF	= -W -Wall -ansi -I ../include
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

libSystem_OBJS= appclient.o appserver.o array.o common.o event.o string.o
libSystem_CFLAGS=$(CFLAGSF) $(CFLAGS)
libSystem.a: $(libSystem_OBJS)
	$(AR) libSystem.a $(libSystem_OBJS)
	$(RANLIB) libSystem.a

libSystem.so: $(libSystem_OBJS)
	$(LD) -o libSystem.so $(libSystem_OBJS)

appclient.o: appclient.c
	$(CC) $(libSystem_CFLAGS) -c appclient.c

appserver.o: appserver.c
	$(CC) $(libSystem_CFLAGS) -c appserver.c

array.o: array.c
	$(CC) $(libSystem_CFLAGS) -c array.c

common.o: common.c
	$(CC) $(libSystem_CFLAGS) -c common.c

event.o: event.c
	$(CC) $(libSystem_CFLAGS) -c event.c

string.o: string.c
	$(CC) $(libSystem_CFLAGS) -c string.c


clean:
	$(RM) $(libSystem_OBJS)

distclean: clean
	$(RM) $(TARGETS)
