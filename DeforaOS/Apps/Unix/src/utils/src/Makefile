TARGETS	= basename dirname false id ln logname true tty unlink
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF=
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

basename: basename.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o basename basename.o

dirname: dirname.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o dirname dirname.o

false: false.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o false false.o

id: id.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o id id.o

ln: ln.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o ln ln.o

logname: logname.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o logname logname.o

true: true.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o true true.o

tty: tty.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o tty tty.o

unlink: unlink.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o unlink unlink.o


basename.o: basename.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c basename.c

dirname.o: dirname.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c dirname.c

false.o: false.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c false.c

id.o: id.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c id.c

ln.o: ln.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c ln.c

logname.o: logname.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c logname.c

true.o: true.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c true.c

tty.o: tty.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c tty.c

unlink.o: unlink.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c unlink.c


clean:
	$(RM) basename.o dirname.o false.o id.o logname.o true.o tty.o unlink.o

distclean: clean
	$(RM) $(TARGETS)
