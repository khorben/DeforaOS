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

libutils.a: config.o file.o slist.o
	$(AR) libutils.a config.o file.o slist.o
	$(RANLIB) libutils.a

libutils.so: config.o file.o slist.o
	$(LD) -o libutils.so config.o file.o slist.o

config.o: config.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c config.c

file.o: file.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c file.c

slist.o: slist.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c slist.c

clean:
	$(RM) config.o file.o slist.o

distclean: clean
	$(RM) libutils.a libutils.so
