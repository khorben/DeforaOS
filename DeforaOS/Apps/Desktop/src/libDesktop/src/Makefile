TARGETS	= libDesktop.a libDesktop.so.0.0 libDesktop.so.0 libDesktop.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../include
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libSystem gtk+-2.0`
CFLAGS	= -Wall -g -O2 -fPIC
LDFLAGSF= `pkg-config --libs libSystem gtk+-2.0`
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

libDesktop_OBJS = about.o accel.o assistant.o menubar.o message.o mime.o toolbar.o
libDesktop_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libDesktop_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

libDesktop.a: $(libDesktop_OBJS)
	$(AR) -rc libDesktop.a $(libDesktop_OBJS)
	$(RANLIB) libDesktop.a

libDesktop.so.0.0 libDesktop.so.0 libDesktop.so: $(libDesktop_OBJS)
	$(CCSHARED) -o libDesktop.so.0.0 -Wl,-soname,libDesktop.so.0 $(libDesktop_OBJS) $(libDesktop_LDFLAGS)
	$(LN) -s -- libDesktop.so.0.0 libDesktop.so.0
	$(LN) -s -- libDesktop.so.0.0 libDesktop.so

about.o: about.c
	$(CC) $(libDesktop_CFLAGS) -c about.c

accel.o: accel.c
	$(CC) $(libDesktop_CFLAGS) -c accel.c

assistant.o: assistant.c
	$(CC) $(libDesktop_CFLAGS) -c assistant.c

menubar.o: menubar.c
	$(CC) $(libDesktop_CFLAGS) -c menubar.c

message.o: message.c
	$(CC) $(libDesktop_CFLAGS) -c message.c

mime.o: mime.c
	$(CC) $(libDesktop_CFLAGS) -c mime.c

toolbar.o: toolbar.c
	$(CC) $(libDesktop_CFLAGS) -c toolbar.c

clean:
	$(RM) -- $(libDesktop_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 0644 -- libDesktop.a $(DESTDIR)$(PREFIX)/lib/libDesktop.a
	$(INSTALL) -m 0755 -- libDesktop.so.0.0 $(DESTDIR)$(PREFIX)/lib/libDesktop.so.0.0
	$(LN) -s -- libDesktop.so.0.0 $(DESTDIR)$(PREFIX)/lib/libDesktop.so.0
	$(LN) -s -- libDesktop.so.0.0 $(DESTDIR)$(PREFIX)/lib/libDesktop.so

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libDesktop.a
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libDesktop.so.0.0
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libDesktop.so.0
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/libDesktop.so

.PHONY: all clean distclean install uninstall
