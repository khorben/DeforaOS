TARGETS	= vesa.a vesa.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -fPIC -pedantic
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

vesa_OBJS = vbe.o vesa.o
vesa_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
vesa_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

vesa.a: $(vesa_OBJS)
	$(AR) vesa.a $(vesa_OBJS)
	$(RANLIB) vesa.a

vesa.so: $(vesa_OBJS)
	$(LD) -o vesa.so -Wl,-soname,vesa.so.0 $(vesa_OBJS)

vbe.o: vbe.c
	$(CC) $(vesa_CFLAGS) -c vbe.c

vesa.o: vesa.c
	$(CC) $(vesa_CFLAGS) -c vesa.c

clean:
	$(RM) -- $(vesa_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
