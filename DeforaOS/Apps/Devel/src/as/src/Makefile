SUBDIRS	= arch format
TARGETS	= as
CC	= cc
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF= -l dl
RM	= rm -f


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

as_OBJS = as.o code.o parser.o scanner.o token.o arch/arch.o format/format.o
as_CFLAGS = $(CFLAGSF) $(CFLAGS)
as: $(as_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o as $(as_OBJS)

as.o: as.c
	$(CC) $(as_CFLAGS) -c as.c

code.o: code.c
	$(CC) $(as_CFLAGS) -c code.c

parser.o: parser.c
	$(CC) $(as_CFLAGS) -c parser.c

scanner.o: scanner.c
	$(CC) $(as_CFLAGS) -c scanner.c

token.o: token.c
	$(CC) $(as_CFLAGS) -c token.c

arch/arch.o: arch/arch.c
	$(CC) $(as_CFLAGS) -c arch/arch.c -o arch/arch.o

format/format.o: format/format.c
	$(CC) $(as_CFLAGS) -c format/format.c -o format/format.o

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) $(as_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) $(as_OBJS)
	$(RM) $(TARGETS)
