TARGETS	= basename cat cmp dirname false kill id link ln logname mkfifo true tty uname unlink
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF=
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

basename: basename.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o basename basename.o

cat: cat.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o cat cat.o

cmp: cmp.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o cmp cmp.o

dirname: dirname.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o dirname dirname.o

false: false.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o false false.o

id: id.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o id id.o

kill: kill.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o kill kill.o

ln: ln.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o ln ln.o

link: link.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o link link.o

logname: logname.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o logname logname.o

mkfifo: mkfifo.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o mkfifo mkfifo.o

true: true.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o true true.o

tty: tty.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o tty tty.o

uname: uname.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o uname uname.o

unlink: unlink.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o unlink unlink.o


basename.o: basename.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c basename.c

cat.o: cat.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c cat.c

cmp.o: cmp.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c cmp.c

dirname.o: dirname.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c dirname.c

false.o: false.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c false.c

id.o: id.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c id.c

kill.o: kill.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c kill.c

link.o: link.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c link.c

logname.o: logname.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c logname.c

mkfifo.o: mkfifo.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c mkfifo.c

true.o: true.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c true.c

tty.o: tty.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c tty.c

uname.o: uname.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c uname.c

unlink.o: unlink.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c unlink.c


clean:
	$(RM) basename.o cat.o cmp.o dirname.o false.o id.o kill.o link.o ln.o logname.o mkfifo.o true.o tty.o uname.o unlink.o

distclean: clean
	$(RM) $(TARGETS)
