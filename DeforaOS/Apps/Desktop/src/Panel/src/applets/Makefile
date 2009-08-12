TARGETS	= clock.so cpu.so desktop.so lock.so logout.so main.so memory.so tasks.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= `pkg-config --cflags gtk+-2.0` -fPIC
CFLAGS	= -Wall -g -O2 -pedantic
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

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
main_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
main_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

main.so: $(main_OBJS)
	$(LD) -o main.so $(main_OBJS)

memory_OBJS = memory.o
memory_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
memory_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

memory.so: $(memory_OBJS)
	$(LD) -o memory.so $(memory_OBJS)

tasks_OBJS = tasks.o
tasks_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
tasks_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

tasks.so: $(tasks_OBJS)
	$(LD) -o tasks.so $(tasks_OBJS)

clock.o: clock.c
	$(CC) $(clock_CFLAGS) -c clock.c

cpu.o: cpu.c
	$(CC) $(cpu_CFLAGS) -c cpu.c

desktop.o: desktop.c
	$(CC) $(desktop_CFLAGS) -c desktop.c

lock.o: lock.c
	$(CC) $(lock_CFLAGS) -c lock.c

logout.o: logout.c
	$(CC) $(logout_CFLAGS) -c logout.c

main.o: main.c
	$(CC) $(main_CFLAGS) -c main.c

memory.o: memory.c
	$(CC) $(memory_CFLAGS) -c memory.c

tasks.o: tasks.c
	$(CC) $(tasks_CFLAGS) -c tasks.c

clean:
	$(RM) $(clock_OBJS) $(cpu_OBJS) $(desktop_OBJS) $(lock_OBJS) $(logout_OBJS) $(main_OBJS) $(memory_OBJS) $(tasks_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 clock.so $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 cpu.so $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so
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
	$(INSTALL) -m 0644 tasks.so $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/lock.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/logout.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/main.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/memory.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so

.PHONY: all clean distclean install uninstall
