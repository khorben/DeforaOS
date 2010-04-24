TARGETS	= phone
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic `pkg-config --cflags gtk+-2.0`
LDFLAGSF= `pkg-config --libs gtk+-2.0`
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

phone_OBJS = callbacks.o gsm.o main.o phone.o
phone_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone: $(phone_OBJS)
	$(CC) -o phone $(phone_OBJS) $(phone_LDFLAGS)

callbacks.o: callbacks.c phone.h callbacks.h
	$(CC) $(phone_CFLAGS) -c callbacks.c

gsm.o: gsm.c phone.h gsm.h
	$(CC) $(phone_CFLAGS) -c gsm.c

main.o: main.c phone.h ../config.h
	$(CC) $(phone_CFLAGS) -c main.c

phone.o: phone.c gsm.h callbacks.h phone.h
	$(CC) $(phone_CFLAGS) -c phone.c

clean:
	$(RM) $(phone_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 phone $(DESTDIR)$(BINDIR)/phone

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/phone

.PHONY: all clean distclean install uninstall
