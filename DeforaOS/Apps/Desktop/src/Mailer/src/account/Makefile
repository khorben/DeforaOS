TARGETS	= mbox.so pop3.so imap4.so nntp.so rss.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -fPIC `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

mbox_OBJS = mbox.o
mbox_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
mbox_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

mbox.so: $(mbox_OBJS)
	$(LD) -o mbox.so $(mbox_OBJS)

pop3_OBJS = pop3.o
pop3_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
pop3_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

pop3.so: $(pop3_OBJS)
	$(LD) -o pop3.so $(pop3_OBJS)

imap4_OBJS = imap4.o
imap4_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
imap4_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

imap4.so: $(imap4_OBJS)
	$(LD) -o imap4.so $(imap4_OBJS)

nntp_OBJS = nntp.o
nntp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
nntp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

nntp.so: $(nntp_OBJS)
	$(LD) -o nntp.so $(nntp_OBJS)

rss_OBJS = rss.o
rss_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
rss_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

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
	$(INSTALL) -m 0644 mbox.so $(DESTDIR)$(LIBDIR)/Mailer/account/mbox.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/account
	$(INSTALL) -m 0644 pop3.so $(DESTDIR)$(LIBDIR)/Mailer/account/pop3.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/account
	$(INSTALL) -m 0644 imap4.so $(DESTDIR)$(LIBDIR)/Mailer/account/imap4.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/account
	$(INSTALL) -m 0644 nntp.so $(DESTDIR)$(LIBDIR)/Mailer/account/nntp.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/account
	$(INSTALL) -m 0644 rss.so $(DESTDIR)$(LIBDIR)/Mailer/account/rss.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/mbox.so
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/pop3.so
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/imap4.so
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/nntp.so
	$(RM) $(DESTDIR)$(LIBDIR)/Mailer/account/rss.so

.PHONY: all clean distclean install uninstall
