TARGETS	= libutils.a libutils.so
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

libutils_OBJS= config.o array.o file.o hash.o slist.o
libutils_CFLAGS=$(CFLAGSF) $(CFLAGS)
libutils.a: $(libutils_OBJS)
	$(AR) libutils.a $(libutils_OBJS)
	$(RANLIB) libutils.a

libutils.so: $(libutils_OBJS)
	$(LD) -o libutils.so $(libutils_OBJS)

config.o: config.c
	$(CC) $(libutils_CFLAGS) -c config.c

array.o: array.c
	$(CC) $(libutils_CFLAGS) -c array.c

file.o: file.c
	$(CC) $(libutils_CFLAGS) -c file.c

hash.o: hash.c
	$(CC) $(libutils_CFLAGS) -c hash.c

slist.o: slist.c
	$(CC) $(libutils_CFLAGS) -c slist.c


clean:
	$(RM) $(libutils_OBJS)

distclean: clean
	$(RM) $(TARGETS)
