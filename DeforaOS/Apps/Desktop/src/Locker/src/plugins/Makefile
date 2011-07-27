TARGETS	= password.so slider.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../../include
CPPFLAGS= 
CFLAGSF	= -W `pkg-config --cflags libSystem gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

password_OBJS = password.o
password_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
password_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

password.so: $(password_OBJS)
	$(LD) -o password.so $(password_OBJS) $(password_LDFLAGS)

slider_OBJS = slider.o
slider_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
slider_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

slider.so: $(slider_OBJS)
	$(LD) -o slider.so $(slider_OBJS) $(slider_LDFLAGS)

password.o: password.c ../../include/Locker.h
	$(CC) $(password_CFLAGS) -c password.c

slider.o: slider.c ../../include/Locker.h
	$(CC) $(slider_CFLAGS) -c slider.c

clean:
	$(RM) -- $(password_OBJS) $(slider_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/plugins
	$(INSTALL) -m 0644 -- password.so $(DESTDIR)$(LIBDIR)/Locker/plugins/password.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/plugins
	$(INSTALL) -m 0644 -- slider.so $(DESTDIR)$(LIBDIR)/Locker/plugins/slider.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/plugins/password.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/plugins/slider.so

.PHONY: all clean distclean install uninstall
