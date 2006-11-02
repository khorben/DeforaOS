TARGETS	= mbox.a mbox.so pop3.a pop3.so imap4.a imap4.so
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

pop3_OBJS = pop3.o
pop3_CFLAGS = $(CFLAGSF) $(CFLAGS)
pop3.a: $(pop3_OBJS)
	$(AR) pop3.a $(pop3_OBJS)
	$(RANLIB) pop3.a
pop3.so: $(pop3_OBJS)
	$(LD) -o pop3.so $(pop3_OBJS)

imap4_OBJS = imap4.o
imap4_CFLAGS = $(CFLAGSF) $(CFLAGS)
imap4.a: $(imap4_OBJS)
	$(AR) imap4.a $(imap4_OBJS)
	$(RANLIB) imap4.a
imap4.so: $(imap4_OBJS)
	$(LD) -o imap4.so $(imap4_OBJS)

mbox.o: mbox.c
	$(CC) $(mbox_CFLAGS) -c mbox.c

pop3.o: pop3.c
	$(CC) $(pop3_CFLAGS) -c pop3.c

imap4.o: imap4.c
	$(CC) $(imap4_CFLAGS) -c imap4.c

clean:
	$(RM) $(mbox_OBJS) $(pop3_OBJS) $(imap4_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 mbox.a $(DESTDIR)$(LIBDIR)/mbox.a
	$(INSTALL) -m 0755 mbox.so $(DESTDIR)$(LIBDIR)/mbox.so
	$(INSTALL) -m 0644 pop3.a $(DESTDIR)$(LIBDIR)/pop3.a
	$(INSTALL) -m 0755 pop3.so $(DESTDIR)$(LIBDIR)/pop3.so
	$(INSTALL) -m 0644 imap4.a $(DESTDIR)$(LIBDIR)/imap4.a
	$(INSTALL) -m 0755 imap4.so $(DESTDIR)$(LIBDIR)/imap4.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/mbox.a
	$(RM) $(DESTDIR)$(LIBDIR)/mbox.so
	$(RM) $(DESTDIR)$(LIBDIR)/pop3.a
	$(RM) $(DESTDIR)$(LIBDIR)/pop3.so
	$(RM) $(DESTDIR)$(LIBDIR)/imap4.a
	$(RM) $(DESTDIR)$(LIBDIR)/imap4.so

.PHONY: all clean distclean install uninstall
