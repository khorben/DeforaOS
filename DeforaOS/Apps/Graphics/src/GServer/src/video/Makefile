TARGETS	= vesa.a vesa.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	= cc
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
AR	= ar -rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

vesa_OBJS = vbe.o vesa.o
vesa_CFLAGS = $(CFLAGSF) $(CFLAGS)
vesa.a: $(vesa_OBJS)
	$(AR) vesa.a $(vesa_OBJS)
	$(RANLIB) vesa.a
vesa.so: $(vesa_OBJS)
	$(LD) -o vesa.so $(vesa_OBJS)

vbe.o: vbe.c
	$(CC) $(vesa_CFLAGS) -c vbe.c

vesa.o: vesa.c
	$(CC) $(vesa_CFLAGS) -c vesa.c

clean:
	$(RM) $(vesa_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 vesa.a $(DESTDIR)$(LIBDIR)/vesa.a
	$(INSTALL) -m 0755 vesa.so $(DESTDIR)$(LIBDIR)/vesa.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/vesa.a
	$(RM) $(DESTDIR)$(LIBDIR)/vesa.so

.PHONY: all clean distclean install uninstall
