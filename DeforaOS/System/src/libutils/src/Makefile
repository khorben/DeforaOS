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

libutils.a: file.o slist.o
	$(AR) libutils.a file.o slist.o
	$(RANLIB) libutils.a

libutils.so: file.o slist.o
	$(LD) -o libutils.so file.o slist.o


file.o: file.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c file.c

slist.o: slist.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c slist.c

clean:
	$(RM) file.o slist.o

distclean: clean
	$(RM) libutils.a libutils.so
