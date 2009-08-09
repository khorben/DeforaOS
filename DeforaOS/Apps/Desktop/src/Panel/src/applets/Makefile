TARGETS	= clock.a clock.so cpu.a cpu.so desktop.a desktop.so lock.a lock.so logout.a logout.so main.a main.so memory.a memory.so tasks.a tasks.so
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

clock.a: $(clock_OBJS)
	$(AR) clock.a $(clock_OBJS)
	$(RANLIB) clock.a

clock.so: $(clock_OBJS)
	$(LD) -o clock.so -Wl,-soname,clock.so.0 $(clock_OBJS)

cpu_OBJS = cpu.o
cpu_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
cpu_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

cpu.a: $(cpu_OBJS)
	$(AR) cpu.a $(cpu_OBJS)
	$(RANLIB) cpu.a

cpu.so: $(cpu_OBJS)
	$(LD) -o cpu.so -Wl,-soname,cpu.so.0 $(cpu_OBJS)

desktop_OBJS = desktop.o
desktop_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
desktop_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

desktop.a: $(desktop_OBJS)
	$(AR) desktop.a $(desktop_OBJS)
	$(RANLIB) desktop.a

desktop.so: $(desktop_OBJS)
	$(LD) -o desktop.so -Wl,-soname,desktop.so.0 $(desktop_OBJS)

lock_OBJS = lock.o
lock_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
lock_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

lock.a: $(lock_OBJS)
	$(AR) lock.a $(lock_OBJS)
	$(RANLIB) lock.a

lock.so: $(lock_OBJS)
	$(LD) -o lock.so -Wl,-soname,lock.so.0 $(lock_OBJS)

logout_OBJS = logout.o
logout_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
logout_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

logout.a: $(logout_OBJS)
	$(AR) logout.a $(logout_OBJS)
	$(RANLIB) logout.a

logout.so: $(logout_OBJS)
	$(LD) -o logout.so -Wl,-soname,logout.so.0 $(logout_OBJS)

main_OBJS = main.o
main_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
main_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

main.a: $(main_OBJS)
	$(AR) main.a $(main_OBJS)
	$(RANLIB) main.a

main.so: $(main_OBJS)
	$(LD) -o main.so -Wl,-soname,main.so.0 $(main_OBJS)

memory_OBJS = memory.o
memory_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
memory_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

memory.a: $(memory_OBJS)
	$(AR) memory.a $(memory_OBJS)
	$(RANLIB) memory.a

memory.so: $(memory_OBJS)
	$(LD) -o memory.so -Wl,-soname,memory.so.0 $(memory_OBJS)

tasks_OBJS = tasks.o
tasks_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
tasks_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

tasks.a: $(tasks_OBJS)
	$(AR) tasks.a $(tasks_OBJS)
	$(RANLIB) tasks.a

tasks.so: $(tasks_OBJS)
	$(LD) -o tasks.so -Wl,-soname,tasks.so.0 $(tasks_OBJS)

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
	$(INSTALL) -m 0644 clock.a $(DESTDIR)$(LIBDIR)/Panel/applets/clock.a
	$(INSTALL) -m 0755 clock.so $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so.0.0
	$(LN) -s clock.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so.0
	$(LN) -s clock.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 cpu.a $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.a
	$(INSTALL) -m 0755 cpu.so $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so.0.0
	$(LN) -s cpu.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so.0
	$(LN) -s cpu.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 desktop.a $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.a
	$(INSTALL) -m 0755 desktop.so $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.so.0.0
	$(LN) -s desktop.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.so.0
	$(LN) -s desktop.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 lock.a $(DESTDIR)$(LIBDIR)/Panel/applets/lock.a
	$(INSTALL) -m 0755 lock.so $(DESTDIR)$(LIBDIR)/Panel/applets/lock.so.0.0
	$(LN) -s lock.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/lock.so.0
	$(LN) -s lock.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/lock.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 logout.a $(DESTDIR)$(LIBDIR)/Panel/applets/logout.a
	$(INSTALL) -m 0755 logout.so $(DESTDIR)$(LIBDIR)/Panel/applets/logout.so.0.0
	$(LN) -s logout.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/logout.so.0
	$(LN) -s logout.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/logout.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 main.a $(DESTDIR)$(LIBDIR)/Panel/applets/main.a
	$(INSTALL) -m 0755 main.so $(DESTDIR)$(LIBDIR)/Panel/applets/main.so.0.0
	$(LN) -s main.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/main.so.0
	$(LN) -s main.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/main.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 memory.a $(DESTDIR)$(LIBDIR)/Panel/applets/memory.a
	$(INSTALL) -m 0755 memory.so $(DESTDIR)$(LIBDIR)/Panel/applets/memory.so.0.0
	$(LN) -s memory.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/memory.so.0
	$(LN) -s memory.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/memory.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Panel/applets
	$(INSTALL) -m 0644 tasks.a $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.a
	$(INSTALL) -m 0755 tasks.so $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so.0.0
	$(LN) -s tasks.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so.0
	$(LN) -s tasks.so.0.0 $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/clock.a
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so.0.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/clock.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.a
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so.0.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/cpu.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.a
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.so.0.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.so.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/desktop.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/lock.a
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/lock.so.0.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/lock.so.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/lock.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/logout.a
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/logout.so.0.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/logout.so.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/logout.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/main.a
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/main.so.0.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/main.so.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/main.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/memory.a
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/memory.so.0.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/memory.so.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/memory.so
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.a
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so.0.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so.0
	$(RM) $(DESTDIR)$(LIBDIR)/Panel/applets/tasks.so

.PHONY: all clean distclean install uninstall
