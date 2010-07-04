TARGETS	= engineering pdu smscrypt
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

engineering_OBJS = engineering.o
engineering_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags gtk+-2.0`
engineering_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs gtk+-2.0` -lSystem -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib

engineering: $(engineering_OBJS)
	$(CC) -o engineering $(engineering_OBJS) $(engineering_LDFLAGS)

pdu_OBJS = pdu.o
pdu_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags glib-2.0`
pdu_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs glib-2.0` -lSystem -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib ../src/command.o ../src/modem.o

pdu: $(pdu_OBJS)
	$(CC) -o pdu $(pdu_OBJS) $(pdu_LDFLAGS)

smscrypt_OBJS = smscrypt.o
smscrypt_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags glib-2.0`
smscrypt_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -lSystem -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib

smscrypt: $(smscrypt_OBJS)
	$(CC) -o smscrypt $(smscrypt_OBJS) $(smscrypt_LDFLAGS)

engineering.o: engineering.c ../include/Phone.h ../src/plugins/engineering.c
	$(CC) $(engineering_CFLAGS) -c engineering.c

pdu.o: pdu.c ../include/Phone.h ../src/gsm.c
	$(CC) $(pdu_CFLAGS) -c pdu.c

smscrypt.o: smscrypt.c ../include/Phone.h ../src/plugins/smscrypt.c
	$(CC) $(smscrypt_CFLAGS) -c smscrypt.c

clean:
	$(RM) -- $(engineering_OBJS) $(pdu_OBJS) $(smscrypt_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
