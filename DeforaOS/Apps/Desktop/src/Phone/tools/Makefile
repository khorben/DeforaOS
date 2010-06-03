TARGETS	= smscrypt
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

smscrypt_OBJS = smscrypt.o
smscrypt_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -I $(PREFIX)/include $(CFLAGSF) $(CFLAGS) `pkg-config --cflags glib-2.0`
smscrypt_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -lSystem -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib

smscrypt: $(smscrypt_OBJS)
	$(CC) -o smscrypt $(smscrypt_OBJS) $(smscrypt_LDFLAGS)

smscrypt.o: smscrypt.c ../include/Phone.h ../src/plugins/smscrypt.c
	$(CC) $(smscrypt_CFLAGS) -c smscrypt.c

clean:
	$(RM) $(smscrypt_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
