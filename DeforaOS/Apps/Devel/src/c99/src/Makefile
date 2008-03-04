TARGETS	= c99
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -ansi -g
LDFLAGSF= -L $(PREFIX)/lib -Wl,-rpath $(PREFIX)/lib -l cpp
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

c99_OBJS = c99.o main.o tokenset.o
c99_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

c99: $(c99_OBJS)
	$(CC) -o c99 $(c99_OBJS) $(LDFLAGSF) $(LDFLAGS)

c99.o: c99.c c99.h tokenset.h
	$(CC) $(c99_CFLAGS) -c c99.c

main.o: main.c c99.h
	$(CC) $(c99_CFLAGS) -c main.c

tokenset.o: tokenset.c c99.h tokenset.h
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
