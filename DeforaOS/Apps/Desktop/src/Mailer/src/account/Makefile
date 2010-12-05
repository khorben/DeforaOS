TARGETS	= imap4.so mbox.so pop3.so nntp.so rss.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../../include -I $(PREFIX)/include
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

imap4_OBJS = imap4.o
imap4_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
imap4_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

imap4.so: $(imap4_OBJS)
	$(LD) -o imap4.so $(imap4_OBJS) $(imap4_LDFLAGS)

mbox_OBJS = mbox.o
mbox_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
mbox_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

mbox.so: $(mbox_OBJS)
	$(LD) -o mbox.so $(mbox_OBJS) $(mbox_LDFLAGS)

pop3_OBJS = pop3.o
pop3_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
pop3_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

pop3.so: $(pop3_OBJS)
	$(LD) -o pop3.so $(pop3_OBJS) $(pop3_LDFLAGS)

nntp_OBJS = nntp.o
nntp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
nntp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

nntp.so: $(nntp_OBJS)
	$(LD) -o nntp.so $(nntp_OBJS) $(nntp_LDFLAGS)

rss_OBJS = rss.o
rss_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
rss_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

rss.so: $(rss_OBJS)
	$(LD) -o rss.so $(rss_OBJS) $(rss_LDFLAGS)

imap4.o: imap4.c ../../include/Mailer.h
	$(CC) $(imap4_CFLAGS) -c imap4.c

mbox.o: mbox.c ../../include/Mailer.h
	$(CC) $(mbox_CFLAGS) -c mbox.c

pop3.o: pop3.c ../../include/Mailer.h
	$(CC) $(pop3_CFLAGS) -c pop3.c

nntp.o: nntp.c ../../include/Mailer.h
	$(CC) $(nntp_CFLAGS) -c nntp.c

rss.o: rss.c ../../include/Mailer.h
	$(CC) $(rss_CFLAGS) -c rss.c

clean:
	$(RM) -- $(imap4_OBJS) $(mbox_OBJS) $(pop3_OBJS) $(nntp_OBJS) $(rss_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/account
	$(INSTALL) -m 0644 -- mbox.so $(DESTDIR)$(LIBDIR)/Mailer/account/mbox.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Mailer/account/mbox.so

.PHONY: all clean distclean install uninstall
