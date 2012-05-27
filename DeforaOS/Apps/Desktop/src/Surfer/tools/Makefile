TARGETS	= helper
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF= -I ../src
CPPFLAGS?=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGS	= 
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

helper_OBJS = helper.o
helper_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D WITH_WEBKIT $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libDesktop webkit-1.0`
helper_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libDesktop webkit-1.0` ../src/ghtml.o

helper: $(helper_OBJS)
	$(CC) -o helper $(helper_OBJS) $(helper_LDFLAGS)

helper.o: helper.c ../src/ghtml.c
	$(CC) $(helper_CFLAGS) -c helper.c

clean:
	$(RM) -- $(helper_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
