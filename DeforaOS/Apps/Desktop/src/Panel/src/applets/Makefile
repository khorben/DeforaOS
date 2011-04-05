TARGETS	= battery.so bluetooth.so clock.so cpu.so cpufreq.so desktop.so gps.so gsm.so keyboard.so lock.so logout.so main.so memory.so pager.so phone.so swap.so systray.so tasks.so usb.so volume.so wpa_supplicant.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libSystem gtk+-2.0` -fPIC
CFLAGS	= -Wall -g -O2 -pedantic
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

battery_OBJS = battery.o
battery_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
battery_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

battery.so: $(battery_OBJS)
	$(LD) -o battery.so $(battery_OBJS) $(battery_LDFLAGS)

bluetooth_OBJS = bluetooth.o
bluetooth_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
bluetooth_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

bluetooth.so: $(bluetooth_OBJS)
	$(LD) -o bluetooth.so $(bluetooth_OBJS) $(bluetooth_LDFLAGS)

clock_OBJS = clock.o
clock_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
clock_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

clock.so: $(clock_OBJS)
	$(LD) -o clock.so $(clock_OBJS) $(clock_LDFLAGS)

cpu_OBJS = cpu.o
cpu_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
cpu_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

cpu.so: $(cpu_OBJS)
	$(LD) -o cpu.so $(cpu_OBJS) $(cpu_LDFLAGS)

cpufreq_OBJS = cpufreq.o
cpufreq_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
cpufreq_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

cpufreq.so: $(cpufreq_OBJS)
	$(LD) -o cpufreq.so $(cpufreq_OBJS) $(cpufreq_LDFLAGS)

desktop_OBJS = desktop.o
desktop_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
desktop_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

desktop.so: $(desktop_OBJS)
	$(LD) -o desktop.so $(desktop_OBJS) $(desktop_LDFLAGS)

gps_OBJS = gps.o
gps_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
gps_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

gps.so: $(gps_OBJS)
	$(LD) -o gps.so $(gps_OBJS) $(gps_LDFLAGS)

gsm_OBJS = gsm.o
gsm_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
gsm_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

gsm.so: $(gsm_OBJS)
	$(LD) -o gsm.so $(gsm_OBJS) $(gsm_LDFLAGS)

keyboard_OBJS = keyboard.o
keyboard_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
keyboard_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

keyboard.so: $(keyboard_OBJS)
	$(LD) -o keyboard.so $(keyboard_OBJS) $(keyboard_LDFLAGS)

lock_OBJS = lock.o
lock_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
lock_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

lock.so: $(lock_OBJS)
	$(LD) -o lock.so $(lock_OBJS) $(lock_LDFLAGS)

logout_OBJS = logout.o
logout_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
logout_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

logout.so: $(logout_OBJS)
	$(LD) -o logout.so $(logout_OBJS) $(logout_LDFLAGS)

main_OBJS = main.o
main_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D PREFIX=\"$(PREFIX)\" $(CFLAGSF) $(CFLAGS)
main_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

main.so: $(main_OBJS)
	$(LD) -o main.so $(main_OBJS) $(main_LDFLAGS)

memory_OBJS = memory.o
memory_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
memory_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

memory.so: $(memory_OBJS)
	$(LD) -o memory.so $(memory_OBJS) $(memory_LDFLAGS)

pager_OBJS = pager.o
pager_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
pager_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

pager.so: $(pager_OBJS)
	$(LD) -o pager.so $(pager_OBJS) $(pager_LDFLAGS)

phone_OBJS = phone.o
phone_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
phone_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

phone.so: $(phone_OBJS)
	$(LD) -o phone.so $(phone_OBJS) $(phone_LDFLAGS)

swap_OBJS = swap.o
swap_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
swap_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

swap.so: $(swap_OBJS)
	$(LD) -o swap.so $(swap_OBJS) $(swap_LDFLAGS)

systray_OBJS = systray.o
systray_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
systray_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

systray.so: $(systray_OBJS)
	$(LD) -o systray.so $(systray_OBJS) $(systray_LDFLAGS)

tasks_OBJS = tasks.o
tasks_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
tasks_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

tasks.so: $(tasks_OBJS)
	$(LD) -o tasks.so $(tasks_OBJS) $(tasks_LDFLAGS)

usb_OBJS = usb.o
usb_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
usb_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

usb.so: $(usb_OBJS)
	$(LD) -o usb.so $(usb_OBJS) $(usb_LDFLAGS)

volume_OBJS = volume.o
volume_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) `pkg-config --cflags alsa` $(CFLAGSF) $(CFLAGS)
volume_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs alsa`

volume.so: $(volume_OBJS)
	$(LD) -o volume.so $(volume_OBJS) $(volume_LDFLAGS)

wpa_supplicant_OBJS = wpa_supplicant.o
wpa_supplicant_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
wpa_supplicant_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

wpa_supplicant.so: $(wpa_supplicant_OBJS)
	$(LD) -o wpa_supplicant.so $(wpa_supplicant_OBJS) $(wpa_supplicant_LDFLAGS)

battery.o: battery.c ../../include/Panel.h
	$(CC) $(battery_CFLAGS) -c battery.c

bluetooth.o: bluetooth.c ../../include/Panel.h
	$(CC) $(bluetooth_CFLAGS) -c bluetooth.c

clock.o: clock.c ../../include/Panel.h
	$(CC) $(clock_CFLAGS) -c clock.c

cpu.o: cpu.c ../../include/Panel.h
	$(CC) $(cpu_CFLAGS) -c cpu.c

cpufreq.o: cpufreq.c ../../include/Panel.h
	$(CC) $(cpufreq_CFLAGS) -c cpufreq.c

desktop.o: desktop.c ../../include/Panel.h
	$(CC) $(desktop_CFLAGS) -c desktop.c

gps.o: gps.c ../../include/Panel.h
	$(CC) $(gps_CFLAGS) -c gps.c

gsm.o: gsm.c ../../include/Panel.h
	$(CC) $(gsm_CFLAGS) -c gsm.c

keyboard.o: keyboard.c ../../include/Panel.h
	$(CC) $(keyboard_CFLAGS) -c keyboard.c

lock.o: lock.c ../../include/Panel.h
	$(CC) $(lock_CFLAGS) -c lock.c

logout.o: logout.c ../../include/Panel.h
	$(CC) $(logout_CFLAGS) -c logout.c

main.o: main.c ../../include/Panel.h
	$(CC) $(main_CFLAGS) -c main.c

memory.o: memory.c ../../include/Panel.h
	$(CC) $(memory_CFLAGS) -c memory.c

pager.o: pager.c ../../include/Panel.h
	$(CC) $(pager_CFLAGS) -c pager.c

phone.o: phone.c ../../include/Panel.h
	$(CC) $(phone_CFLAGS) -c phone.c

swap.o: swap.c ../../include/Panel.h
	$(CC) $(swap_CFLAGS) -c swap.c

systray.o: systray.c ../../include/Panel.h
	$(CC) $(systray_CFLAGS) -c systray.c

tasks.o: tasks.c ../../include/Panel.h tasks.atoms
	$(CC) $(tasks_CFLAGS) -c tasks.c

usb.o: usb.c ../../include/Panel.h
	$(CC) $(usb_CFLAGS) -c usb.c

volume.o: volume.c ../../include/Panel.h
	$(CC) $(volume_CFLAGS) -c volume.c

wpa_supplicant.o: wpa_supplicant.c ../../include/Panel.h
	$(CC) $(wpa_supplicant_CFLAGS) -c wpa_supplicant.c

clean:
	$(RM) -- $(battery_OBJS) $(bluetooth_OBJS) $(clock_OBJS) $(cpu_OBJS) $(cpufreq_OBJS) $(desktop_OBJS) $(gps_OBJS) $(gsm_OBJS) $(keyboard_OBJS) $(lock_OBJS) $(logout_OBJS) $(main_OBJS) $(memory_OBJS) $(pager_OBJS) $(phone_OBJS) $(swap_OBJS) $(systray_OBJS) $(tasks_OBJS) $(usb_OBJS) $(volume_OBJS) $(wpa_supplicant_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- battery.so $(DESTDIR)$(LIBDIR)/Panel/applets/battery.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- bluetooth.so $(DESTDIR)$(LIBDIR)/Panel/applets/bluetooth.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- clock.so $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- cpu.so $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- cpufreq.so $(DESTDIR)$(LIBDIR)/Panel/applets/cpufreq.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- desktop.so $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- gps.so $(DESTDIR)$(LIBDIR)/Panel/applets/gps.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- gsm.so $(DESTDIR)$(LIBDIR)/Panel/applets/gsm.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- keyboard.so $(DESTDIR)$(LIBDIR)/Panel/applets/keyboard.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- lock.so $(DESTDIR)$(LIBDIR)/Panel/applets/lock.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- logout.so $(DESTDIR)$(LIBDIR)/Panel/applets/logout.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- main.so $(DESTDIR)$(LIBDIR)/Panel/applets/main.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- memory.so $(DESTDIR)$(LIBDIR)/Panel/applets/memory.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- pager.so $(DESTDIR)$(LIBDIR)/Panel/applets/pager.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- phone.so $(DESTDIR)$(LIBDIR)/Panel/applets/phone.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- swap.so $(DESTDIR)$(LIBDIR)/Panel/applets/swap.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- systray.so $(DESTDIR)$(LIBDIR)/Panel/applets/systray.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- tasks.so $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- usb.so $(DESTDIR)$(LIBDIR)/Panel/applets/usb.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- volume.so $(DESTDIR)$(LIBDIR)/Panel/applets/volume.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 -- wpa_supplicant.so $(DESTDIR)$(LIBDIR)/Panel/applets/wpa_supplicant.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/battery.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/bluetooth.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/cpufreq.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/gps.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/gsm.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/keyboard.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/lock.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/logout.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/main.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/memory.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/pager.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/phone.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/swap.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/systray.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/usb.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/volume.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Panel/applets/wpa_supplicant.so

.PHONY: all clean distclean install uninstall
