TARGETS	= battery.so clock.so cpu.so cpufreq.so desktop.so lock.so logout.so main.so memory.so pager.so tasks.so volume.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I $(PREFIX)/include -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0` -fPIC
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
	$(LD) -o battery.so $(battery_OBJS)

clock_OBJS = clock.o
clock_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
clock_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

clock.so: $(clock_OBJS)
	$(LD) -o clock.so $(clock_OBJS)

cpu_OBJS = cpu.o
cpu_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
cpu_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

cpu.so: $(cpu_OBJS)
	$(LD) -o cpu.so $(cpu_OBJS)

cpufreq_OBJS = cpufreq.o
cpufreq_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
cpufreq_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

cpufreq.so: $(cpufreq_OBJS)
	$(LD) -o cpufreq.so $(cpufreq_OBJS)

desktop_OBJS = desktop.o
desktop_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
desktop_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

desktop.so: $(desktop_OBJS)
	$(LD) -o desktop.so $(desktop_OBJS)

lock_OBJS = lock.o
lock_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
lock_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

lock.so: $(lock_OBJS)
	$(LD) -o lock.so $(lock_OBJS)

logout_OBJS = logout.o
logout_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
logout_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

logout.so: $(logout_OBJS)
	$(LD) -o logout.so $(logout_OBJS)

main_OBJS = main.o
main_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D PREFIX=\"$(PREFIX)\" $(CFLAGSF) $(CFLAGS)
main_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

main.so: $(main_OBJS)
	$(LD) -o main.so $(main_OBJS)

memory_OBJS = memory.o
memory_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
memory_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

memory.so: $(memory_OBJS)
	$(LD) -o memory.so $(memory_OBJS)

pager_OBJS = pager.o
pager_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
pager_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

pager.so: $(pager_OBJS)
	$(LD) -o pager.so $(pager_OBJS)

tasks_OBJS = tasks.o
tasks_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
tasks_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

tasks.so: $(tasks_OBJS)
	$(LD) -o tasks.so $(tasks_OBJS)

volume_OBJS = volume.o
volume_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
volume_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

volume.so: $(volume_OBJS)
	$(LD) -o volume.so $(volume_OBJS)

battery.o: battery.c ../../include/Panel.h
	$(CC) $(battery_CFLAGS) -c battery.c

clock.o: clock.c ../../include/Panel.h
	$(CC) $(clock_CFLAGS) -c clock.c

cpu.o: cpu.c ../../include/Panel.h
	$(CC) $(cpu_CFLAGS) -c cpu.c

cpufreq.o: cpufreq.c ../../include/Panel.h
	$(CC) $(cpufreq_CFLAGS) -c cpufreq.c

desktop.o: desktop.c ../../include/Panel.h
	$(CC) $(desktop_CFLAGS) -c desktop.c

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

tasks.o: tasks.c ../../include/Panel.h tasks.atoms
	$(CC) $(tasks_CFLAGS) -c tasks.c

volume.o: volume.c ../../include/Panel.h
	$(CC) $(volume_CFLAGS) -c volume.c

clean:
	$(RM) $(battery_OBJS) $(clock_OBJS) $(cpu_OBJS) $(cpufreq_OBJS) $(desktop_OBJS) $(lock_OBJS) $(logout_OBJS) $(main_OBJS) $(memory_OBJS) $(pager_OBJS) $(tasks_OBJS) $(volume_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 battery.so $(DESTDIR)$(LIBDIR)/Panel/applets/battery.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 clock.so $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 cpu.so $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 cpufreq.so $(DESTDIR)$(LIBDIR)/Panel/applets/cpufreq.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 desktop.so $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 lock.so $(DESTDIR)$(LIBDIR)/Panel/applets/lock.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 logout.so $(DESTDIR)$(LIBDIR)/Panel/applets/logout.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 main.so $(DESTDIR)$(LIBDIR)/Panel/applets/main.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 memory.so $(DESTDIR)$(LIBDIR)/Panel/applets/memory.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 pager.so $(DESTDIR)$(LIBDIR)/Panel/applets/pager.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 tasks.so $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 volume.so $(DESTDIR)$(LIBDIR)/Panel/applets/volume.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/battery.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/cpufreq.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/lock.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/logout.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/main.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/memory.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/pager.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/volume.so

.PHONY: all clean distclean install uninstall
