TARGETS	= mbox.a mbox.so pop3.a pop3.so imap4.a imap4.so rss.a rss.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CFLAGSF	= -W -fPIC
CFLAGS	= -Wall -g -O2
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

rss_OBJS = rss.o
rss_CFLAGS = $(CFLAGSF) $(CFLAGS)
rss.a: $(rss_OBJS)
	$(AR) rss.a $(rss_OBJS)
	$(RANLIB) rss.a
rss.so: $(rss_OBJS)
	$(LD) -o rss.so $(rss_OBJS)

mbox.o: mbox.c account.h
	$(CC) $(mbox_CFLAGS) -c mbox.c

pop3.o: pop3.c account.h
	$(CC) $(pop3_CFLAGS) -c pop3.c

imap4.o: imap4.c account.h
	$(CC) $(imap4_CFLAGS) -c imap4.c

rss.o: rss.c account.h
	$(CC) $(rss_CFLAGS) -c rss.c

clean:
	$(RM) $(mbox_OBJS) $(pop3_OBJS) $(imap4_OBJS) $(rss_OBJS)

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
	$(INSTALL) -m 0644 rss.a $(DESTDIR)$(LIBDIR)/rss.a
	$(INSTALL) -m 0755 rss.so $(DESTDIR)$(LIBDIR)/rss.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/mbox.a
	$(RM) $(DESTDIR)$(LIBDIR)/mbox.so
	$(RM) $(DESTDIR)$(LIBDIR)/pop3.a
	$(RM) $(DESTDIR)$(LIBDIR)/pop3.so
	$(RM) $(DESTDIR)$(LIBDIR)/imap4.a
	$(RM) $(DESTDIR)$(LIBDIR)/imap4.so
	$(RM) $(DESTDIR)$(LIBDIR)/rss.a
	$(RM) $(DESTDIR)$(LIBDIR)/rss.so

.PHONY: all clean distclean install uninstall
