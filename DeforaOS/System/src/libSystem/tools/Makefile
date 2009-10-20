TARGETS	= broker
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= -lSystem -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

broker_OBJS = broker.o
broker_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
broker_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

broker: $(broker_OBJS)
	$(CC) -o broker $(broker_OBJS) $(broker_LDFLAGS)

broker.o: broker.c
	$(CC) $(broker_CFLAGS) -c broker.c

clean:
	$(RM) $(broker_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
