TARGETS	= strace
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

strace_OBJS = linux.o strace.o
strace_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
strace_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

strace: $(strace_OBJS)
	$(CC) -o strace $(strace_OBJS) $(strace_LDFLAGS)

linux.o: linux.c
	$(CC) $(strace_CFLAGS) -c linux.c

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
