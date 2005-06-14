TARGETS	= libSystem.a libSystem.so
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

libSystem_OBJS= appclient.o appserver.o
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


clean:
	$(RM) $(libSystem_OBJS)

distclean: clean
	$(RM) $(TARGETS)
