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

libutils.a: config.o darray.o file.o hash.o slist.o
	$(AR) libutils.a config.o darray.o file.o hash.o slist.o
	$(RANLIB) libutils.a

libutils.so: config.o darray.o file.o hash.o slist.o
	$(LD) -o libutils.so config.o darray.o file.o hash.o slist.o

config.o: config.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c config.c

darray.o: darray.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c darray.c

file.o: file.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c file.c

hash.o: hash.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c hash.c

slist.o: slist.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c slist.c

clean:
	$(RM) config.o darray.o file.o hash.o slist.o

distclean: clean
	$(RM) libutils.a libutils.so
