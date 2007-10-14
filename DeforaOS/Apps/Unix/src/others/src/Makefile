TARGETS	= hexdump login mktemp mount reboot tar umount uptime w
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -ansi
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

hexdump_OBJS = hexdump.o
hexdump_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

hexdump: $(hexdump_OBJS)
	$(CC) -o hexdump $(hexdump_OBJS) $(LDFLAGSF) $(LDFLAGS)

login_OBJS = login.o
login_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

login: $(login_OBJS)
	$(CC) -o login $(login_OBJS) $(LDFLAGSF) $(LDFLAGS)

mktemp_OBJS = mktemp.o
mktemp_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

mktemp: $(mktemp_OBJS)
	$(CC) -o mktemp $(mktemp_OBJS) $(LDFLAGSF) $(LDFLAGS)

mount_OBJS = mount.o
mount_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

mount: $(mount_OBJS)
	$(CC) -o mount $(mount_OBJS) $(LDFLAGSF) $(LDFLAGS)

reboot_OBJS = reboot.o
reboot_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

reboot: $(reboot_OBJS)
	$(CC) -o reboot $(reboot_OBJS) $(LDFLAGSF) $(LDFLAGS)

tar_OBJS = tar.o
tar_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

tar: $(tar_OBJS)
	$(CC) -o tar $(tar_OBJS) $(LDFLAGSF) $(LDFLAGS)

umount_OBJS = umount.o
umount_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

umount: $(umount_OBJS)
	$(CC) -o umount $(umount_OBJS) $(LDFLAGSF) $(LDFLAGS)

uptime_OBJS = uptime.o
uptime_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

uptime: $(uptime_OBJS)
	$(CC) -o uptime $(uptime_OBJS) $(LDFLAGSF) $(LDFLAGS)

w_OBJS = w.o
w_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

w: $(w_OBJS)
	$(CC) -o w $(w_OBJS) $(LDFLAGSF) $(LDFLAGS)

hexdump.o: hexdump.c
	$(CC) $(hexdump_CFLAGS) -c hexdump.c

login.o: login.c
	$(CC) $(login_CFLAGS) -c login.c

mktemp.o: mktemp.c
	$(CC) $(mktemp_CFLAGS) -c mktemp.c

mount.o: mount.c
	$(CC) $(mount_CFLAGS) -c mount.c

reboot.o: reboot.c
	$(CC) $(reboot_CFLAGS) -c reboot.c

tar.o: tar.c tar.h
	$(CC) $(tar_CFLAGS) -c tar.c

umount.o: umount.c
	$(CC) $(umount_CFLAGS) -c umount.c

uptime.o: uptime.c
	$(CC) $(uptime_CFLAGS) -c uptime.c

w.o: w.c
	$(CC) $(w_CFLAGS) -c w.c

clean:
	$(RM) $(hexdump_OBJS) $(login_OBJS) $(mktemp_OBJS) $(mount_OBJS) $(reboot_OBJS) $(tar_OBJS) $(umount_OBJS) $(uptime_OBJS) $(w_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 hexdump $(DESTDIR)$(BINDIR)/hexdump
	$(INSTALL) -m 0755 login $(DESTDIR)$(BINDIR)/login
	$(INSTALL) -m 0755 mktemp $(DESTDIR)$(BINDIR)/mktemp
	$(INSTALL) -m 0755 mount $(DESTDIR)$(BINDIR)/mount
	$(INSTALL) -m 0755 reboot $(DESTDIR)$(BINDIR)/reboot
	$(INSTALL) -m 0755 tar $(DESTDIR)$(BINDIR)/tar
	$(INSTALL) -m 0755 umount $(DESTDIR)$(BINDIR)/umount
	$(INSTALL) -m 0755 uptime $(DESTDIR)$(BINDIR)/uptime
	$(INSTALL) -m 0755 w $(DESTDIR)$(BINDIR)/w

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/hexdump
	$(RM) $(DESTDIR)$(BINDIR)/login
	$(RM) $(DESTDIR)$(BINDIR)/mktemp
	$(RM) $(DESTDIR)$(BINDIR)/mount
	$(RM) $(DESTDIR)$(BINDIR)/reboot
	$(RM) $(DESTDIR)$(BINDIR)/tar
	$(RM) $(DESTDIR)$(BINDIR)/umount
	$(RM) $(DESTDIR)$(BINDIR)/uptime
	$(RM) $(DESTDIR)$(BINDIR)/w

.PHONY: all clean distclean install uninstall
