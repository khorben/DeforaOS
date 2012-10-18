TARGETS	= hello struct union
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= ../src/c99
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W
CFLAGS	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

hello_OBJS = hello
hello_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

struct_OBJS = struct
struct_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

union_OBJS = union
union_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

hello: hello.c
	$(CC) $(hello_CFLAGS) -c hello.c

struct: struct.c
	$(CC) $(struct_CFLAGS) -c struct.c

union: union.c
	$(CC) $(union_CFLAGS) -c union.c

clean:
	$(RM) -- $(hello_OBJS) $(struct_OBJS) $(union_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
