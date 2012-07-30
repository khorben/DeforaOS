TARGETS	= todo
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libSystem libDesktop`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem libDesktop`
LDFLAGS	= 
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

todo_OBJS = task.o taskedit.o todo.o callbacks.o main.o
todo_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
todo_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

todo: $(todo_OBJS)
	$(CC) -o todo $(todo_OBJS) $(todo_LDFLAGS)

task.o: task.c task.h
	$(CC) $(todo_CFLAGS) -c task.c

taskedit.o: taskedit.c
	$(CC) $(todo_CFLAGS) -c taskedit.c

todo.o: todo.c callbacks.h task.h todo.h ../config.h
	$(CC) $(todo_CFLAGS) -c todo.c

callbacks.o: callbacks.c task.h todo.h callbacks.h
	$(CC) $(todo_CFLAGS) -c callbacks.c

main.o: main.c task.h todo.h ../config.h
	$(CC) $(todo_CFLAGS) -c main.c

clean:
	$(RM) -- $(todo_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- todo $(DESTDIR)$(BINDIR)/todo

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/todo

.PHONY: all clean distclean install uninstall
