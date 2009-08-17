TARGETS	= as.so graph.so indent.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF=
CPPFLAGS= -I ../../include
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -fPIC -pedantic
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

as_OBJS = as.o
as_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
as_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L $(LIBDIR) -Wl,-rpath $(LIBDIR) -l as

as.so: $(as_OBJS)
	$(LD) -o as.so $(as_OBJS) -L $(LIBDIR) -Wl,-rpath $(LIBDIR) -l as

graph_OBJS = graph.o
graph_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
graph_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

graph.so: $(graph_OBJS)
	$(LD) -o graph.so $(graph_OBJS)

indent_OBJS = indent.o
indent_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
indent_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

indent.so: $(indent_OBJS)
	$(LD) -o indent.so $(indent_OBJS)

as.o: as.c ../../include/c99/target.h ../../config.h
	$(CC) $(as_CFLAGS) -c as.c

graph.o: graph.c ../../include/c99/target.h
	$(CC) $(graph_CFLAGS) -c graph.c

indent.o: indent.c ../../include/c99/target.h
	$(CC) $(indent_CFLAGS) -c indent.c

clean:
	$(RM) $(as_OBJS) $(graph_OBJS) $(indent_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/c99/target
	$(INSTALL) -m 0644 as.so $(DESTDIR)$(LIBDIR)/c99/target/as.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/c99/target
	$(INSTALL) -m 0644 graph.so $(DESTDIR)$(LIBDIR)/c99/target/graph.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/c99/target
	$(INSTALL) -m 0644 indent.so $(DESTDIR)$(LIBDIR)/c99/target/indent.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/c99/target/as.so
	$(RM) $(DESTDIR)$(LIBDIR)/c99/target/graph.so
	$(RM) $(DESTDIR)$(LIBDIR)/c99/target/indent.so

.PHONY: all clean distclean install uninstall
