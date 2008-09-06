TARGETS	= halt hexdump hostname login mktemp mount poweroff reboot tar umount uptime w
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -ansi
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

halt_OBJS = halt.o
halt_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

halt: $(halt_OBJS)
	$(CC) -o halt $(halt_OBJS) $(LDFLAGSF) $(LDFLAGS)

hexdump_OBJS = hexdump.o
hexdump_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

hexdump: $(hexdump_OBJS)
	$(CC) -o hexdump $(hexdump_OBJS) $(LDFLAGSF) $(LDFLAGS)

hostname_OBJS = hostname.o
hostname_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

hostname: $(hostname_OBJS)
	$(CC) -o hostname $(hostname_OBJS) $(LDFLAGSF) $(LDFLAGS)

login_OBJS = login.o
login_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

login: $(login_OBJS)
	$(CC) -o login $(login_OBJS) $(LDFLAGSF) $(LDFLAGS)

mktemp_OBJS = mktemp.o
mktemp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

mktemp: $(mktemp_OBJS)
	$(CC) -o mktemp $(mktemp_OBJS) $(LDFLAGSF) $(LDFLAGS)

mount_OBJS = mount.o
mount_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

mount: $(mount_OBJS)
	$(CC) -o mount $(mount_OBJS) $(LDFLAGSF) $(LDFLAGS)

poweroff_OBJS = poweroff.o
poweroff_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

poweroff: $(poweroff_OBJS)
	$(CC) -o poweroff $(poweroff_OBJS) $(LDFLAGSF) $(LDFLAGS)

reboot_OBJS = reboot.o
reboot_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

reboot: $(reboot_OBJS)
	$(CC) -o reboot $(reboot_OBJS) $(LDFLAGSF) $(LDFLAGS)

tar_OBJS = tar.o
tar_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

tar: $(tar_OBJS)
	$(CC) -o tar $(tar_OBJS) $(LDFLAGSF) $(LDFLAGS)

umount_OBJS = umount.o
umount_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

umount: $(umount_OBJS)
	$(CC) -o umount $(umount_OBJS) $(LDFLAGSF) $(LDFLAGS)

uptime_OBJS = uptime.o
uptime_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

uptime: $(uptime_OBJS)
	$(CC) -o uptime $(uptime_OBJS) $(LDFLAGSF) $(LDFLAGS)

w_OBJS = w.o
w_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

w: $(w_OBJS)
	$(CC) -o w $(w_OBJS) $(LDFLAGSF) $(LDFLAGS)

halt.o: halt.c
	$(CC) $(halt_CFLAGS) -c halt.c

hexdump.o: hexdump.c
	$(CC) $(hexdump_CFLAGS) -c hexdump.c

hostname.o: hostname.c
	$(CC) $(hostname_CFLAGS) -c hostname.c

login.o: login.c
	$(CC) $(login_CFLAGS) -c login.c

mktemp.o: mktemp.c
	$(CC) $(mktemp_CFLAGS) -c mktemp.c

mount.o: mount.c
	$(CC) $(mount_CFLAGS) -c mount.c

poweroff.o: poweroff.c
	$(CC) $(poweroff_CFLAGS) -c poweroff.c

reboot.o: reboot.c
	$(CC) $(reboot_CFLAGS) -c reboot.c

tar.o: tar.c tar.h
	$(CC) $(tar_CFLAGS) -c tar.c

umount.o: umount.c
	$(CC) $(umount_CFLAGS) -c umount.c

uptime.o: uptime.c utmpx.c
	$(CC) $(uptime_CFLAGS) -c uptime.c

w.o: w.c utmpx.c
	$(CC) $(w_CFLAGS) -c w.c

clean:
	$(RM) $(halt_OBJS) $(hexdump_OBJS) $(hostname_OBJS) $(login_OBJS) $(mktemp_OBJS) $(mount_OBJS) $(poweroff_OBJS) $(reboot_OBJS) $(tar_OBJS) $(umount_OBJS) $(uptime_OBJS) $(w_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 halt $(DESTDIR)$(BINDIR)/halt
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 hexdump $(DESTDIR)$(BINDIR)/hexdump
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 hostname $(DESTDIR)$(BINDIR)/hostname
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 login $(DESTDIR)$(BINDIR)/login
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 mktemp $(DESTDIR)$(BINDIR)/mktemp
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 mount $(DESTDIR)$(BINDIR)/mount
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 poweroff $(DESTDIR)$(BINDIR)/poweroff
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 reboot $(DESTDIR)$(BINDIR)/reboot
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 tar $(DESTDIR)$(BINDIR)/tar
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 umount $(DESTDIR)$(BINDIR)/umount
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 uptime $(DESTDIR)$(BINDIR)/uptime
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 w $(DESTDIR)$(BINDIR)/w

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/halt
	$(RM) $(DESTDIR)$(BINDIR)/hexdump
	$(RM) $(DESTDIR)$(BINDIR)/hostname
	$(RM) $(DESTDIR)$(BINDIR)/login
	$(RM) $(DESTDIR)$(BINDIR)/mktemp
	$(RM) $(DESTDIR)$(BINDIR)/mount
	$(RM) $(DESTDIR)$(BINDIR)/poweroff
	$(RM) $(DESTDIR)$(BINDIR)/reboot
	$(RM) $(DESTDIR)$(BINDIR)/tar
	$(RM) $(DESTDIR)$(BINDIR)/umount
	$(RM) $(DESTDIR)$(BINDIR)/uptime
	$(RM) $(DESTDIR)$(BINDIR)/w

.PHONY: all clean distclean install uninstall
