TARGETS	= ar
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CFLAGSF	= -W -Wall
CFLAGS	= -g
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

ar_OBJS = ar.o
ar_CFLAGS = $(CFLAGSF) $(CFLAGS)
ar: $(ar_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o ar $(ar_OBJS)

ar.o: ar.c
	$(CC) $(ar_CFLAGS) -c ar.c

clean:
	$(RM) $(ar_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 ar $(DESTDIR)$(BINDIR)/ar

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/ar
