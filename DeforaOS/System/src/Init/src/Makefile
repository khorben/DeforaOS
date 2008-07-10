TARGETS	= Init Service
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGS=
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -l System -Wl,--export-dynamic 
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

Init_OBJS = init.o service.o
Init_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

Init: $(Init_OBJS)
	$(CC) -o Init $(Init_OBJS) $(LDFLAGSF) $(LDFLAGS)

Service_OBJS = main.o
Service_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

Service: $(Service_OBJS)
	$(CC) -o Service $(Service_OBJS) $(LDFLAGSF) $(LDFLAGS)

init.o: init.c service.h ../config.h
	$(CC) $(Init_CFLAGS) -c init.c

service.o: service.c service.h
	$(CC) $(Init_CFLAGS) -c service.c

main.o: main.c
	$(CC) $(Service_CFLAGS) -c main.c

clean:
	$(RM) $(Init_OBJS) $(Service_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 Init $(DESTDIR)$(BINDIR)/Init

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/Init

.PHONY: all clean distclean install uninstall
