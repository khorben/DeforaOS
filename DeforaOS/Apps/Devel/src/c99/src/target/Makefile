TARGETS	= asm.so graph.so indent.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags cpp`
CFLAGS	= -Wall -g -O2 -fPIC -pedantic
LDFLAGSF= `pkg-config --libs cpp`
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

asm_OBJS = asm.o
asm_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags Asm`
asm_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs Asm`

asm.so: $(asm_OBJS)
	$(CCSHARED) -o asm.so $(asm_OBJS) $(asm_LDFLAGS)

graph_OBJS = graph.o
graph_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
graph_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

graph.so: $(graph_OBJS)
	$(CCSHARED) -o graph.so $(graph_OBJS) $(graph_LDFLAGS)

indent_OBJS = indent.o
indent_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
indent_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

indent.so: $(indent_OBJS)
	$(CCSHARED) -o indent.so $(indent_OBJS) $(indent_LDFLAGS)

asm.o: asm.c ../../include/C99/target.h asm/amd64.c ../../config.h
	$(CC) $(asm_CFLAGS) -c asm.c

graph.o: graph.c ../../include/C99/target.h
	$(CC) $(graph_CFLAGS) -c graph.c

indent.o: indent.c ../../include/C99/target.h
	$(CC) $(indent_CFLAGS) -c indent.c

clean:
	$(RM) -- $(asm_OBJS) $(graph_OBJS) $(indent_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/C99/target
	$(INSTALL) -m 0644 -- asm.so $(DESTDIR)$(LIBDIR)/C99/target/asm.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/C99/target
	$(INSTALL) -m 0644 -- graph.so $(DESTDIR)$(LIBDIR)/C99/target/graph.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/C99/target
	$(INSTALL) -m 0644 -- indent.so $(DESTDIR)$(LIBDIR)/C99/target/indent.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/C99/target/asm.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/C99/target/graph.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/C99/target/indent.so

.PHONY: all clean distclean install uninstall
