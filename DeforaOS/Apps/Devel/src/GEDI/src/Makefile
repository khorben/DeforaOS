TARGETS	= gedi
CC	= cc
CFLAGSF	= -W -Wall -ansi `pkg-config --cflags gtk+-2.0` -I /System/Include
CFLAGS	= -g
LDFLAGSF= `pkg-config --libs gtk+-2.0` -L /System/Libraries -l System
RM	= rm -f


all: $(TARGETS)

gedi_OBJS = gedi.o main.o project.o
gedi_CFLAGS = $(CFLAGSF) $(CFLAGS)
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
