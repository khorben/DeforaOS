TARGETS	= glut.so glx.so vesa.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libApp`
CFLAGS	= -Wall -g -O2 -fPIC -pedantic
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

glut_OBJS = glut.o
glut_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
glut_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs glut`

glut.so: $(glut_OBJS)
	$(CCSHARED) -o glut.so $(glut_OBJS) $(glut_LDFLAGS)

glx_OBJS = glx.o
glx_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
glx_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs glu`

glx.so: $(glx_OBJS)
	$(CCSHARED) -o glx.so $(glx_OBJS) $(glx_LDFLAGS)

vesa_OBJS = vbe.o vesa.o
vesa_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
vesa_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

vesa.so: $(vesa_OBJS)
	$(CCSHARED) -o vesa.so $(vesa_OBJS) $(vesa_LDFLAGS)

glut.o: glut.c video.h
	$(CC) $(glut_CFLAGS) `pkg-config --cflags glut` -c glut.c

glx.o: glx.c video.h
	$(CC) $(glx_CFLAGS) `pkg-config --cflags glu` -c glx.c

vbe.o: vbe.c
	$(CC) $(vesa_CFLAGS) -c vbe.c

vesa.o: vesa.c video.h
	$(CC) $(vesa_CFLAGS) -c vesa.c

clean:
	$(RM) -- $(glut_OBJS) $(glx_OBJS) $(vesa_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(PREFIX)/lib/GServer/video
	$(INSTALL) -m 0644 -- glut.so $(DESTDIR)$(PREFIX)/lib/GServer/video/glut.so
	$(MKDIR) $(DESTDIR)$(PREFIX)/lib/GServer/video
	$(INSTALL) -m 0644 -- glx.so $(DESTDIR)$(PREFIX)/lib/GServer/video/glx.so
	$(MKDIR) $(DESTDIR)$(PREFIX)/lib/GServer/video
	$(INSTALL) -m 0644 -- vesa.so $(DESTDIR)$(PREFIX)/lib/GServer/video/vesa.so

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/GServer/video/glut.so
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/GServer/video/glx.so
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/GServer/video/vesa.so

.PHONY: all clean distclean install uninstall
