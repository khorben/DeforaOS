TARGETS	= mbox.a mbox.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
AR	= ar -rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

mbox_OBJS = mbox.o
mbox_CFLAGS = $(CFLAGSF) $(CFLAGS)
mbox.a: $(mbox_OBJS)
	$(AR) mbox.a $(mbox_OBJS)
	$(RANLIB) mbox.a
mbox.so: $(mbox_OBJS)
	$(LD) -o mbox.so $(mbox_OBJS)

mbox.o: mbox.c
	$(CC) $(mbox_CFLAGS) -c mbox.c

clean:
	$(RM) $(mbox_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 mbox.a $(DESTDIR)$(LIBDIR)/mbox.a
	$(INSTALL) -m 0755 mbox.so $(DESTDIR)$(LIBDIR)/mbox.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/mbox.a
	$(RM) $(DESTDIR)$(LIBDIR)/mbox.so

.PHONY: all clean distclean install uninstall
