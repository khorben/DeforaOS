TARGETS	= cpp
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CFLAGSF	= -W -Wall
CFLAGS	= -g
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

cpp_OBJS = cpp.o
cpp_CFLAGS = $(CFLAGSF) $(CFLAGS)
cpp: $(cpp_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o cpp $(cpp_OBJS)

cpp.o: cpp.c
	$(CC) $(cpp_CFLAGS) -c cpp.c

clean:
	$(RM) $(cpp_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 cpp $(DESTDIR)$(BINDIR)/cpp

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/cpp

.PHONY: all clean distclean install uninstall
