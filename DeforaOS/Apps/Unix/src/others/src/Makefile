TARGETS	= halt hexdump host hostname login mktemp mount poweroff reboot tar umount uptime w
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

halt_OBJS = halt.o
halt_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
halt_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

halt: $(halt_OBJS)
	$(CC) -o halt $(halt_OBJS) $(halt_LDFLAGS)

hexdump_OBJS = hexdump.o
hexdump_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
hexdump_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

hexdump: $(hexdump_OBJS)
	$(CC) -o hexdump $(hexdump_OBJS) $(hexdump_LDFLAGS)

host_OBJS = host.o
host_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
host_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) 

host: $(host_OBJS)
	$(CC) -o host $(host_OBJS) $(host_LDFLAGS)

hostname_OBJS = hostname.o
hostname_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
hostname_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

hostname: $(hostname_OBJS)
	$(CC) -o hostname $(hostname_OBJS) $(hostname_LDFLAGS)

login_OBJS = login.o
login_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
login_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

login: $(login_OBJS)
	$(CC) -o login $(login_OBJS) $(login_LDFLAGS)

mktemp_OBJS = mktemp.o
mktemp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
mktemp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

mktemp: $(mktemp_OBJS)
	$(CC) -o mktemp $(mktemp_OBJS) $(mktemp_LDFLAGS)

mount_OBJS = mount.o
mount_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
mount_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

mount: $(mount_OBJS)
	$(CC) -o mount $(mount_OBJS) $(mount_LDFLAGS)

poweroff_OBJS = poweroff.o
poweroff_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
poweroff_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

poweroff: $(poweroff_OBJS)
	$(CC) -o poweroff $(poweroff_OBJS) $(poweroff_LDFLAGS)

reboot_OBJS = reboot.o
reboot_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
reboot_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

reboot: $(reboot_OBJS)
	$(CC) -o reboot $(reboot_OBJS) $(reboot_LDFLAGS)

tar_OBJS = tar.o
tar_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
tar_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

tar: $(tar_OBJS)
	$(CC) -o tar $(tar_OBJS) $(tar_LDFLAGS)

umount_OBJS = umount.o
umount_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
umount_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

umount: $(umount_OBJS)
	$(CC) -o umount $(umount_OBJS) $(umount_LDFLAGS)

uptime_OBJS = uptime.o
uptime_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
uptime_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

uptime: $(uptime_OBJS)
	$(CC) -o uptime $(uptime_OBJS) $(uptime_LDFLAGS)

w_OBJS = w.o
w_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
w_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

w: $(w_OBJS)
	$(CC) -o w $(w_OBJS) $(w_LDFLAGS)

halt.o: halt.c
	$(CC) $(halt_CFLAGS) -c halt.c

hexdump.o: hexdump.c
	$(CC) $(hexdump_CFLAGS) -c hexdump.c

host.o: host.c
	$(CC) $(host_CFLAGS) -c host.c

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
	$(RM) $(halt_OBJS) $(hexdump_OBJS) $(host_OBJS) $(hostname_OBJS) $(login_OBJS) $(mktemp_OBJS) $(mount_OBJS) $(poweroff_OBJS) $(reboot_OBJS) $(tar_OBJS) $(umount_OBJS) $(uptime_OBJS) $(w_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 halt $(DESTDIR)$(BINDIR)/halt
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 hexdump $(DESTDIR)$(BINDIR)/hexdump
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 host $(DESTDIR)$(BINDIR)/host
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
	$(RM) $(DESTDIR)$(BINDIR)/host
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
