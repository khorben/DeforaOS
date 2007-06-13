TARGETS	= strace
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CPPFLAGS=
CFLAGSF	= -W -Wall -ansi
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

strace_OBJS = strace.o
strace_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

strace: $(strace_OBJS)
	$(CC) -o strace $(strace_OBJS) $(LDFLAGSF) $(LDFLAGS)

strace.o: strace.c
	$(CC) $(strace_CFLAGS) -c strace.c

clean:
	$(RM) $(strace_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 strace $(DESTDIR)$(BINDIR)/strace

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/strace

.PHONY: all clean distclean install uninstall
