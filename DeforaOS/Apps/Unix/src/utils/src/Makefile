TARGETS	= basename dirname false id logname true
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

logname: logname.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o logname logname.o

true: true.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o true true.o


false.o: false.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c false.c

id.o: id.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c id.c

logname.o: logname.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c logname.c

true.o: true.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c true.c


clean:
	$(RM) false.o true.o

distclean: clean
	$(RM) $(TARGETS)
