TARGETS	= password.so slider.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libSystem gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
LDFLAGSF= -W `pkg-config --libs libSystem gtk+-2.0`
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

password_OBJS = password.o
password_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
password_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -l crypt

password.so: $(password_OBJS)
	$(CCSHARED) -o password.so $(password_OBJS) $(password_LDFLAGS)

slider_OBJS = slider.o
slider_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
slider_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

slider.so: $(slider_OBJS)
	$(CCSHARED) -o slider.so $(slider_OBJS) $(slider_LDFLAGS)

password.o: password.c ../../include/Locker.h ../../config.h
	$(CC) $(password_CFLAGS) -c password.c

slider.o: slider.c ../../include/Locker.h
	$(CC) $(slider_CFLAGS) -c slider.c

clean:
	$(RM) -- $(password_OBJS) $(slider_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/auth
	$(INSTALL) -m 0644 -- password.so $(DESTDIR)$(LIBDIR)/Locker/auth/password.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/auth
	$(INSTALL) -m 0644 -- slider.so $(DESTDIR)$(LIBDIR)/Locker/auth/slider.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/auth/password.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/auth/slider.so

.PHONY: all clean distclean install uninstall
