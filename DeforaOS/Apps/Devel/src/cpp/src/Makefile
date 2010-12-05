TARGETS	= libcpp.a libcpp.so cpp
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I $(PREFIX)/include -I ../include
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
BINDIR	= $(PREFIX)/bin
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libcpp_OBJS = cpp.o parser.o scanner.o
libcpp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -fPIC
libcpp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L$(LIBDIR) -Wl,-rpath,$(LIBDIR) -lSystem

libcpp.a: $(libcpp_OBJS)
	$(AR) libcpp.a $(libcpp_OBJS)
	$(RANLIB) libcpp.a

libcpp.so: $(libcpp_OBJS)
	$(LD) -o libcpp.so -Wl,-soname,libcpp.so.0 $(libcpp_OBJS) $(libcpp_LDFLAGS)

cpp_OBJS = main.o
cpp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
cpp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L. -L$(LIBDIR) -Wl,-rpath,$(LIBDIR) -lcpp

cpp: $(cpp_OBJS) libcpp.so
	$(CC) -o cpp $(cpp_OBJS) $(cpp_LDFLAGS)

cpp.o: cpp.c common.h ../include/cpp.h
	$(CC) $(libcpp_CFLAGS) -c cpp.c

parser.o: parser.c parser.h ../include/cpp.h
	$(CC) $(libcpp_CFLAGS) -c parser.c

scanner.o: scanner.c common.h ../include/cpp.h
	$(CC) $(libcpp_CFLAGS) -c scanner.c

main.o: main.c ../include/cpp.h ../config.h
	$(CC) $(cpp_CFLAGS) -c main.c

clean:
	$(RM) -- $(libcpp_OBJS) $(cpp_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 -- libcpp.a $(DESTDIR)$(LIBDIR)/libcpp.a
	$(INSTALL) -m 0755 -- libcpp.so $(DESTDIR)$(LIBDIR)/libcpp.so.0.0
	$(LN) -s -- libcpp.so.0.0 $(DESTDIR)$(LIBDIR)/libcpp.so.0
	$(LN) -s -- libcpp.so.0.0 $(DESTDIR)$(LIBDIR)/libcpp.so
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- cpp $(DESTDIR)$(BINDIR)/cpp

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/libcpp.a
	$(RM) -- $(DESTDIR)$(LIBDIR)/libcpp.so.0.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libcpp.so.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libcpp.so
	$(RM) -- $(DESTDIR)$(BINDIR)/cpp

.PHONY: all clean distclean install uninstall
