TARGETS	= c99
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CFLAGSF	= -W
CFLAGS	= -g
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

c99_OBJS = c99.o
c99_CFLAGS = $(CFLAGSF) $(CFLAGS)
c99: $(c99_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o c99 $(c99_OBJS)

c99.o: c99.c
	$(CC) $(c99_CFLAGS) -c c99.c

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
