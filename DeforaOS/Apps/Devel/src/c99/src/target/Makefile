TARGETS	= asm.so graph.so indent.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags cpp`
CFLAGS	= -Wall -g -O2 -fPIC -pedantic
LDFLAGSF= `pkg-config --libs cpp`
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

asm_OBJS = asm.o
asm_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags asm`
asm_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs asm`

asm.so: $(asm_OBJS)
	$(LD) -o asm.so $(asm_OBJS) $(asm_LDFLAGS)

graph_OBJS = graph.o
graph_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
graph_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

graph.so: $(graph_OBJS)
	$(LD) -o graph.so $(graph_OBJS) $(graph_LDFLAGS)

indent_OBJS = indent.o
indent_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
indent_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

indent.so: $(indent_OBJS)
	$(LD) -o indent.so $(indent_OBJS) $(indent_LDFLAGS)

asm.o: asm.c ../../include/c99/target.h ../../config.h
	$(CC) $(asm_CFLAGS) -c asm.c

graph.o: graph.c ../../include/c99/target.h
	$(CC) $(graph_CFLAGS) -c graph.c

indent.o: indent.c ../../include/c99/target.h
	$(CC) $(indent_CFLAGS) -c indent.c

clean:
	$(RM) -- $(asm_OBJS) $(graph_OBJS) $(indent_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/c99/target
	$(INSTALL) -m 0644 -- asm.so $(DESTDIR)$(LIBDIR)/c99/target/asm.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/c99/target
	$(INSTALL) -m 0644 -- graph.so $(DESTDIR)$(LIBDIR)/c99/target/graph.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/c99/target
	$(INSTALL) -m 0644 -- indent.so $(DESTDIR)$(LIBDIR)/c99/target/indent.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/c99/target/asm.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/c99/target/graph.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/c99/target/indent.so

.PHONY: all clean distclean install uninstall
