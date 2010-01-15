TARGETS	= ../data/Directory.h Directory
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I $(PREFIX)/include
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem -Wl,--export-dynamic
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

../data/Directory.h: ../data/Directory.interface
	./appbroker.sh "../data/Directory.h"

Directory_OBJS = directory.o main.o
Directory_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
Directory_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

Directory: $(Directory_OBJS)
	$(CC) -o Directory $(Directory_OBJS) $(Directory_LDFLAGS)

directory.o: directory.c ../data/Directory.h directory.h
	$(CC) $(Directory_CFLAGS) -c directory.c

main.o: main.c directory.h
	$(CC) $(Directory_CFLAGS) -c main.c

clean:
	$(RM) $(../data/Directory.h_OBJS) $(Directory_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 Directory $(DESTDIR)$(BINDIR)/Directory

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/Directory

.PHONY: all clean distclean install uninstall
