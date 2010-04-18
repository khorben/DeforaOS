TARGETS	= gedi
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I $(PREFIX)/include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config --libs gtk+-2.0` -lSystem -lDesktop
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

gedi_OBJS = callbacks.o gedi.o main.o project.o
gedi_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
gedi_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

gedi: $(gedi_OBJS)
	$(CC) -o gedi $(gedi_OBJS) $(gedi_LDFLAGS)

callbacks.o: callbacks.c ../config.h callbacks.h gedi.h
	$(CC) $(gedi_CFLAGS) -c callbacks.c

gedi.o: gedi.c ../config.h callbacks.h gedi.h
	$(CC) $(gedi_CFLAGS) -c gedi.c

main.o: main.c
	$(CC) $(gedi_CFLAGS) -c main.c

project.o: project.c
	$(CC) $(gedi_CFLAGS) -c project.c

clean:
	$(RM) $(gedi_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 gedi $(DESTDIR)$(BINDIR)/gedi

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/gedi

.PHONY: all clean distclean install uninstall
