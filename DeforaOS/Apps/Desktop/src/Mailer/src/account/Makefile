TARGETS	= mbox.a mbox.so pop3.a pop3.so imap4.a imap4.so nntp.a nntp.so rss.a rss.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -fPIC `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

mbox_OBJS = mbox.o
mbox_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

mbox.a: $(mbox_OBJS)
	$(AR) mbox.a $(mbox_OBJS)
	$(RANLIB) mbox.a

mbox.so: $(mbox_OBJS)
	$(LD) -o mbox.so $(mbox_OBJS)

pop3_OBJS = pop3.o
pop3_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

pop3.a: $(pop3_OBJS)
	$(AR) pop3.a $(pop3_OBJS)
	$(RANLIB) pop3.a

pop3.so: $(pop3_OBJS)
	$(LD) -o pop3.so $(pop3_OBJS)

imap4_OBJS = imap4.o
imap4_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

imap4.a: $(imap4_OBJS)
	$(AR) imap4.a $(imap4_OBJS)
	$(RANLIB) imap4.a

imap4.so: $(imap4_OBJS)
	$(LD) -o imap4.so $(imap4_OBJS)

nntp_OBJS = nntp.o
nntp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

nntp.a: $(nntp_OBJS)
	$(AR) nntp.a $(nntp_OBJS)
	$(RANLIB) nntp.a

nntp.so: $(nntp_OBJS)
	$(LD) -o nntp.so $(nntp_OBJS)

rss_OBJS = rss.o
rss_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

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

nntp.o: nntp.c account.h
	$(CC) $(nntp_CFLAGS) -c nntp.c

rss.o: rss.c account.h
	$(CC) $(rss_CFLAGS) -c rss.c

clean:
	$(RM) $(mbox_OBJS) $(pop3_OBJS) $(imap4_OBJS) $(nntp_OBJS) $(rss_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/account
	$(INSTALL) -m 0644 mbox.a $(DESTDIR)$(LIBDIR)/Mailer/account/mbox.a
	$(INSTALL) -m 0755 mbox.so $(DESTDIR)$(LIBDIR)/Mailer/account/mbox.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/account
	$(INSTALL) -m 0644 pop3.a $(DESTDIR)$(LIBDIR)/Mailer/account/pop3.a
	$(INSTALL) -m 0755 pop3.so $(DESTDIR)$(LIBDIR)/Mailer/account/pop3.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/account
	$(INSTALL) -m 0644 imap4.a $(DESTDIR)$(LIBDIR)/Mailer/account/imap4.a
	$(INSTALL) -m 0755 imap4.so $(DESTDIR)$(LIBDIR)/Mailer/account/imap4.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/account
	$(INSTALL) -m 0644 nntp.a $(DESTDIR)$(LIBDIR)/Mailer/account/nntp.a
	$(INSTALL) -m 0755 nntp.so $(DESTDIR)$(LIBDIR)/Mailer/account/nntp.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/account
	$(INSTALL) -m 0644 rss.a $(DESTDIR)$(LIBDIR)/Mailer/account/rss.a
	$(INSTALL) -m 0755 rss.so $(DESTDIR)$(LIBDIR)/Mailer/account/rss.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/mbox.a
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/mbox.so
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/pop3.a
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/pop3.so
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/imap4.a
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/imap4.so
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/nntp.a
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/nntp.so
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/rss.a
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/rss.so

.PHONY: all clean distclean install uninstall
