TARGETS	= c99
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGS= -I ../include
CFLAGSF	= -W
CFLAGS	= -Wall -ansi -g
LDFLAGSF= -L $(PREFIX)/lib -Wl,-rpath $(PREFIX)/lib -l cpp
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

c99_OBJS = c99.o main.o parser.o scanner.o tokenset.o
c99_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

c99: $(c99_OBJS)
	$(CC) -o c99 $(c99_OBJS) $(LDFLAGSF) $(LDFLAGS)

c99.o: c99.c ../include/c99.h common.h
	$(CC) $(c99_CFLAGS) -c c99.c

main.o: main.c ../include/c99.h
	$(CC) $(c99_CFLAGS) -c main.c

parser.o: parser.c tokenset.h common.h
	$(CC) $(c99_CFLAGS) -c parser.c

scanner.o: scanner.c
	$(CC) $(c99_CFLAGS) -c scanner.c

tokenset.o: tokenset.c ../include/c99.h common.h tokenset.h
	$(CC) $(c99_CFLAGS) -c tokenset.c

clean:
	$(RM) $(c99_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 c99 $(DESTDIR)$(BINDIR)/c99

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/c99

.PHONY: all clean distclean install uninstall
