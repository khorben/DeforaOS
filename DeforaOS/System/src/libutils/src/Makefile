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

libutils.a: file.o
	$(AR) libutils.a file.o
	$(RANLIB) libutils.a

libutils.so: file.o
	$(LD) -o libutils.so file.o


file.o: file.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c file.c


clean:
	$(RM) file.o

distclean: clean
	$(RM) libutils.a libutils.so
