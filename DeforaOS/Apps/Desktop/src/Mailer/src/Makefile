TARGETS	= mailer
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CFLAGSF	= -W `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -Wall -g -O2 -ansi
LDFLAGSF= `pkg-config gtk+-2.0 --libs` -l System
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

mailer_OBJS = callbacks.o common.o compose.o mailer.o main.o
mailer_CFLAGS = $(CFLAGSF) $(CFLAGS)
mailer: $(mailer_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o mailer $(mailer_OBJS)

callbacks.o: callbacks.c callbacks.h compose.h mailer.h
	$(CC) $(mailer_CFLAGS) -c callbacks.c

common.o: common.c
	$(CC) $(mailer_CFLAGS) -c common.c

compose.o: compose.c callbacks.h mailer.h compose.h
	$(CC) $(mailer_CFLAGS) -c compose.c

mailer.o: mailer.c callbacks.h compose.h mailer.h
	$(CC) $(mailer_CFLAGS) -c mailer.c

main.o: main.c mailer.h
	$(CC) $(mailer_CFLAGS) -c main.c

clean:
	$(RM) $(mailer_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 mailer $(DESTDIR)$(BINDIR)/mailer

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/mailer

.PHONY: all clean distclean install uninstall
