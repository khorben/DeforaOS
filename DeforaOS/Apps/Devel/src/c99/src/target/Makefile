TARGETS	= as.a as.so graph.a graph.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF=
CPPFLAGS= -I ../../include
CFLAGSF	= -W
CFLAGS	= -Wall -g -fPIC
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

as_OBJS = as.o
as_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
as_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L $(LIBDIR) -Wl,-rpath $(LIBDIR) -l as

as.a: $(as_OBJS)
	$(AR) as.a $(as_OBJS)
	$(RANLIB) as.a

as.so: $(as_OBJS)
	$(LD) -o as.so $(as_OBJS) -L $(LIBDIR) -Wl,-rpath $(LIBDIR) -l as

graph_OBJS = graph.o
graph_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
graph_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

graph.a: $(graph_OBJS)
	$(AR) graph.a $(graph_OBJS)
	$(RANLIB) graph.a

graph.so: $(graph_OBJS)
	$(LD) -o graph.so $(graph_OBJS)

as.o: as.c ../../include/c99/target.h ../../config.h
	$(CC) $(as_CFLAGS) -c as.c

graph.o: graph.c ../../include/c99/target.h
	$(CC) $(graph_CFLAGS) -c graph.c

clean:
	$(RM) $(as_OBJS) $(graph_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/c99/target
	$(INSTALL) -m 0644 as.a $(DESTDIR)$(LIBDIR)/c99/target/as.a
	$(INSTALL) -m 0755 as.so $(DESTDIR)$(LIBDIR)/c99/target/as.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/c99/target
	$(INSTALL) -m 0644 graph.a $(DESTDIR)$(LIBDIR)/c99/target/graph.a
	$(INSTALL) -m 0755 graph.so $(DESTDIR)$(LIBDIR)/c99/target/graph.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/c99/target/as.a
	$(RM) $(DESTDIR)$(LIBDIR)/c99/target/as.so
	$(RM) $(DESTDIR)$(LIBDIR)/c99/target/graph.a
	$(RM) $(DESTDIR)$(LIBDIR)/c99/target/graph.so

.PHONY: all clean distclean install uninstall
