TARGETS	= i386.a i386.so i486.a i486.so sparc.a sparc.so
CFLAGSF	= -W -Wall -ansi
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

i386_OBJS= i386.o
i386_CFLAGS=$(CFLAGSF) $(CFLAGS)
i386.a: $(i386_OBJS)
	$(AR) i386.a $(i386_OBJS)
	$(RANLIB) i386.a

i386.so: $(i386_OBJS)
	$(LD) -o i386.so $(i386_OBJS)

i386.o: i386.c
	$(CC) $(i386_CFLAGS) -c i386.c

i486_OBJS= i486.o
i486_CFLAGS=$(CFLAGSF) $(CFLAGS)
i486.a: $(i486_OBJS)
	$(AR) i486.a $(i486_OBJS)
	$(RANLIB) i486.a

i486.so: $(i486_OBJS)
	$(LD) -o i486.so $(i486_OBJS)

i486.o: i486.c
	$(CC) $(i486_CFLAGS) -c i486.c

sparc_OBJS= sparc.o
sparc_CFLAGS=$(CFLAGSF) $(CFLAGS)
sparc.a: $(sparc_OBJS)
	$(AR) sparc.a $(sparc_OBJS)
	$(RANLIB) sparc.a

sparc.so: $(sparc_OBJS)
	$(LD) -o sparc.so $(sparc_OBJS)

sparc.o: sparc.c
	$(CC) $(sparc_CFLAGS) -c sparc.c


clean:
	$(RM) $(i386_OBJS) $(i486_OBJS) $(sparc_OBJS)

distclean: clean
	$(RM) $(TARGETS)
