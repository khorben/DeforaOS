TARGETS	= libutils
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

libutils: libutils.a libutils.so

LIBUTILS_OBJS = array.o config.o file.o hash.o slist.o
libutils.a: $(LIBUTILS_OBJS)
	$(AR) libutils.a $(LIBUTILS_OBJS)
	$(RANLIB) libutils.a

libutils.so: $(LIBUTILS_OBJS)
	$(LD) -o libutils.so $(LIBUTILS_OBJS)

array.o: array.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c array.c

config.o: config.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c config.c

file.o: file.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c file.c

hash.o: hash.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c hash.c

slist.o: slist.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c slist.c

clean:
	$(RM) $(LIBUTILS_OBJS)

distclean: clean
	$(RM) libutils.a libutils.so
