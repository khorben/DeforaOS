TARGETS	= basename cat cmp dirname false head id kill link ln logname ls mkdir mkfifo rmdir true tty uname unlink
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

head: head.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o head head.o

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

ls: ls.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o ls ls.o

mkdir: mkdir.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o mkdir mkdir.o

mkfifo: mkfifo.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o mkfifo mkfifo.o

rmdir: rmdir.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o rmdir rmdir.o

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

head.o: head.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c head.c

id.o: id.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c id.c

kill.o: kill.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c kill.c

link.o: link.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c link.c

ln.o: ln.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c ln.c

logname.o: logname.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c logname.c

ls.o: ls.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c ls.c

mkdir.o: mkdir.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c mkdir.c

mkfifo.o: mkfifo.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c mkfifo.c

rmdir.o: rmdir.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c rmdir.c

true.o: true.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c true.c

tty.o: tty.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c tty.c

uname.o: uname.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c uname.c

unlink.o: unlink.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c unlink.c


clean:
	$(RM) basename.o cat.o cmp.o dirname.o false.o head.o id.o kill.o link.o ln.o logname.o ls.o mkdir.o mkfifo.o rmdir.o true.o tty.o uname.o unlink.o

distclean: clean
	$(RM) $(TARGETS)
