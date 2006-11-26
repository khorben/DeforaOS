TARGETS	= surfer
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0 firefox-gtkmozembed`
CFLAGS	= -Wall -g -O2 -ansi
LDFLAGSF= `pkg-config --libs gtk+-2.0 firefox-gtkmozembed`
LDFLAGS	= 
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

surfer_OBJS = surfer.o callbacks.o main.o
surfer_CFLAGS = $(CFLAGSF) $(CFLAGS)
surfer: $(surfer_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o surfer $(surfer_OBJS)

surfer.o: surfer.c callbacks.h surfer.h ../config.h
	$(CC) $(surfer_CFLAGS) -c surfer.c

callbacks.o: callbacks.c surfer.h callbacks.h
	$(CC) $(surfer_CFLAGS) -c callbacks.c

main.o: main.c surfer.h
	$(CC) $(surfer_CFLAGS) -c main.c

clean:
	$(RM) $(surfer_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 surfer $(DESTDIR)$(BINDIR)/surfer

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/surfer

.PHONY: all clean distclean install uninstall
