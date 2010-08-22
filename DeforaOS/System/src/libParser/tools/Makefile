TARGETS	= xml
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I../include
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

xml_OBJS = xml.o
xml_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
xml_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L ../src -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lParser

xml: $(xml_OBJS)
	$(CC) -o xml $(xml_OBJS) $(xml_LDFLAGS)

xml.o: xml.c
	$(CC) $(xml_CFLAGS) -c xml.c

clean:
	$(RM) -- $(xml_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
