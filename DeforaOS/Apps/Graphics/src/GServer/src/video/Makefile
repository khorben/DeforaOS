TARGETS	= glx.so vesa.so
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

glx_OBJS = glx.o
glx_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
glx_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs glu`

glx.so: $(glx_OBJS)
	$(LD) -o glx.so $(glx_OBJS) `pkg-config --libs glu`

vesa_OBJS = vbe.o vesa.o
vesa_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
vesa_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

vesa.so: $(vesa_OBJS)
	$(LD) -o vesa.so $(vesa_OBJS)

glx.o: glx.c video.h
	$(CC) $(glx_CFLAGS) `pkg-config --cflags glu` -c glx.c

vbe.o: vbe.c
	$(CC) $(vesa_CFLAGS) -c vbe.c

vesa.o: vesa.c video.h
	$(CC) $(vesa_CFLAGS) -c vesa.c

clean:
	$(RM) -- $(glx_OBJS) $(vesa_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(PREFIX)/lib/GServer/video
	$(INSTALL) -m 0644 -- glx.so $(DESTDIR)$(PREFIX)/lib/GServer/video/glx.so
	$(MKDIR) $(DESTDIR)$(PREFIX)/lib/GServer/video
	$(INSTALL) -m 0644 -- vesa.so $(DESTDIR)$(PREFIX)/lib/GServer/video/vesa.so

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/GServer/video/glx.so
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/GServer/video/vesa.so

.PHONY: all clean distclean install uninstall
