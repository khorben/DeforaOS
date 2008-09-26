TARGETS	= Directory
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -l System -Wl,--export-dynamic 
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

Directory_OBJS = directory.o main.o
Directory_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

Directory: $(Directory_OBJS)
	$(CC) -o Directory $(Directory_OBJS) $(LDFLAGSF) $(LDFLAGS)

directory.o: directory.c directory.h
	$(CC) $(Directory_CFLAGS) -c directory.c

main.o: main.c directory.h
	$(CC) $(Directory_CFLAGS) -c main.c

clean:
	$(RM) $(Directory_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 Directory $(DESTDIR)$(BINDIR)/Directory

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/Directory

.PHONY: all clean distclean install uninstall
