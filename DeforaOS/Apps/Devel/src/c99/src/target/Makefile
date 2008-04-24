TARGETS	= amd64.a amd64.so graph.a graph.so sparc64.a sparc64.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
AR	= ar -rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

amd64_OBJS = amd64.o
amd64_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

amd64.a: $(amd64_OBJS)
	$(AR) amd64.a $(amd64_OBJS)
	$(RANLIB) amd64.a

amd64.so: $(amd64_OBJS)
	$(LD) -o amd64.so $(amd64_OBJS)

graph_OBJS = graph.o
graph_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

graph.a: $(graph_OBJS)
	$(AR) graph.a $(graph_OBJS)
	$(RANLIB) graph.a

graph.so: $(graph_OBJS)
	$(LD) -o graph.so $(graph_OBJS)

sparc64_OBJS = sparc64.o
sparc64_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

sparc64.a: $(sparc64_OBJS)
	$(AR) sparc64.a $(sparc64_OBJS)
	$(RANLIB) sparc64.a

sparc64.so: $(sparc64_OBJS)
	$(LD) -o sparc64.so $(sparc64_OBJS)

amd64.o: amd64.c
	$(CC) $(amd64_CFLAGS) -c amd64.c

graph.o: graph.c
	$(CC) $(graph_CFLAGS) -c graph.c

sparc64.o: sparc64.c
	$(CC) $(sparc64_CFLAGS) -c sparc64.c

clean:
	$(RM) $(amd64_OBJS) $(graph_OBJS) $(sparc64_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 amd64.a $(DESTDIR)$(LIBDIR)/amd64.a
	$(INSTALL) -m 0755 amd64.so $(DESTDIR)$(LIBDIR)/amd64.so
	$(INSTALL) -m 0644 graph.a $(DESTDIR)$(LIBDIR)/graph.a
	$(INSTALL) -m 0755 graph.so $(DESTDIR)$(LIBDIR)/graph.so
	$(INSTALL) -m 0644 sparc64.a $(DESTDIR)$(LIBDIR)/sparc64.a
	$(INSTALL) -m 0755 sparc64.so $(DESTDIR)$(LIBDIR)/sparc64.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/amd64.a
	$(RM) $(DESTDIR)$(LIBDIR)/amd64.so
	$(RM) $(DESTDIR)$(LIBDIR)/graph.a
	$(RM) $(DESTDIR)$(LIBDIR)/graph.so
	$(RM) $(DESTDIR)$(LIBDIR)/sparc64.a
	$(RM) $(DESTDIR)$(LIBDIR)/sparc64.so

.PHONY: all clean distclean install uninstall
