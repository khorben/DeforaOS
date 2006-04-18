TARGETS	= libGToolkit.a libGToolkit.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	= cc
CFLAGSF	= -W -Wall -ansi -D_GNU_SOURCE
CFLAGS	= -g
AR	= ar -rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libGToolkit_OBJS = gtoolkit.o
libGToolkit_CFLAGS = $(CFLAGSF) $(CFLAGS)
libGToolkit.a: $(libGToolkit_OBJS)
	$(AR) libGToolkit.a $(libGToolkit_OBJS)
	$(RANLIB) libGToolkit.a
libGToolkit.so: $(libGToolkit_OBJS)
	$(LD) -o libGToolkit.so $(libGToolkit_OBJS)

gtoolkit.o: gtoolkit.c
	$(CC) $(libGToolkit_CFLAGS) -c gtoolkit.c

clean:
	$(RM) $(libGToolkit_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 libGToolkit.a $(DESTDIR)$(LIBDIR)/libGToolkit.a
	$(INSTALL) -m 0755 libGToolkit.so $(DESTDIR)$(LIBDIR)/libGToolkit.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/libGToolkit.a
	$(RM) $(DESTDIR)$(LIBDIR)/libGToolkit.so
