TARGETS	= x86.a x86.so
CFLAGSF	= -W -Wall -ansi
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

x86_OBJS= x86.o
x86_CFLAGS=$(CFLAGSF) $(CFLAGS)
x86.a: $(x86_OBJS)
	$(AR) x86.a $(x86_OBJS)
	$(RANLIB) x86.a

x86.so: $(x86_OBJS)
	$(LD) -o x86.so $(x86_OBJS)

x86.o: x86.c
	$(CC) $(x86_CFLAGS) -c x86.c


clean:
	$(RM) $(x86_OBJS)

distclean: clean
	$(RM) $(TARGETS)
