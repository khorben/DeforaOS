TARGETS	= todo
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I $(PREFIX)/include
CPPFLAGS=
CFLAGSF	= -W `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem -lDesktop `pkg-config gtk+-2.0 --libs`
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

todo_OBJS = todo.o callbacks.o main.o
todo_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
todo_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

todo: $(todo_OBJS)
	$(CC) -o todo $(todo_OBJS) $(todo_LDFLAGS)

todo.o: todo.c callbacks.h todo.h ../config.h
	$(CC) $(todo_CFLAGS) -c todo.c

callbacks.o: callbacks.c todo.h callbacks.h
	$(CC) $(todo_CFLAGS) -c callbacks.c

main.o: main.c todo.h ../config.h
	$(CC) $(todo_CFLAGS) -c main.c

clean:
	$(RM) $(todo_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 todo $(DESTDIR)$(BINDIR)/todo

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/todo

.PHONY: all clean distclean install uninstall
