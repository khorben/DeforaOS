TARGETS	= libGToolkit.a libGToolkit.so
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

libGToolkit_OBJS= gtoolkit.o
libGToolkit_CFLAGS=$(CFLAGSF) $(CFLAGS)
libGToolkit.a: $(libGToolkit_OBJS)
	$(AR) libGToolkit.a $(libGToolkit_OBJS)
	$(RANLIB) libGToolkit.a

libGToolkit.so: $(libGToolkit_OBJS)
	$(LD) -o libGToolkit.so $(libGToolkit_OBJS)

gtoolkit.o: gtoolkit.c
	$(CC) $(libGToolkit_CFLAGS) -c gtoolkit.c


clean:
	$(RM) $(libGToolkit_OBJS)

distclean: clean
	$(RM) $(TARGETS)
