TARGETS	= libGToolkit
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

libGToolkit_OBJS= gtoolkit.o
libGToolkit: libGToolkit.a libGToolkit.so
libGToolkit.a: $(libGToolkit_OBJS)
	$(AR) libGToolkit.a $(libGToolkit_OBJS)
	$(RANLIB) libGToolkit.a

libGToolkit.so: $(libGToolkit_OBJS)
	$(LD) -o libGToolkit.so $(libGToolkit_OBJS)

gtoolkit.o: gtoolkit.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c gtoolkit.c


clean:
	$(RM) $(libGToolkit_OBJS)

distclean: clean
	$(RM) $(TARGETS)
