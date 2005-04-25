TARGETS	= gedi
CFLAGSF	= `pkg-config --cflags gtk+-2.0` -I /usr/local/include/libutils
CFLAGS	= -g
LDFLAGSF= `pkg-config --libs gtk+-2.0` -l utils
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

gedi_OBJS= gedi.o main.o project.o
gedi_CFLAGS=$(CFLAGSF) $(CFLAGS)
gedi: $(gedi_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o gedi $(gedi_OBJS)

gedi.o: gedi.c
	$(CC) $(gedi_CFLAGS) -c gedi.c

main.o: main.c
	$(CC) $(gedi_CFLAGS) -c main.c

project.o: project.c
	$(CC) $(gedi_CFLAGS) -c project.c


clean:
	$(RM) $(gedi_OBJS)

distclean: clean
	$(RM) $(TARGETS)
