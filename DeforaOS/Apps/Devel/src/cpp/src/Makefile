TARGETS	= libcpp.a libcpp.so cpp
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGS= -I ../include
CFLAGSF	= -W -Wall
CFLAGS	= -g
AR	= ar -rc
RANLIB	= ranlib
LD	= ld -shared
BINDIR	= $(PREFIX)/bin
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libcpp_OBJS = cpp.o
libcpp_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -fPIC

libcpp.a: $(libcpp_OBJS)
	$(AR) libcpp.a $(libcpp_OBJS)
	$(RANLIB) libcpp.a

libcpp.so: $(libcpp_OBJS)
	$(LD) -o libcpp.so $(libcpp_OBJS) -L $(LIBDIR) -rpath $(LIBDIR) -l System

cpp_OBJS = main.o
cpp_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

cpp: $(cpp_OBJS)
	$(CC) -o cpp $(cpp_OBJS) $(LDFLAGSF) $(LDFLAGS) -Wl,-rpath,$(LIBDIR) -L $(LIBDIR) -L . -l cpp -l System -l ssl

cpp.o: cpp.c ../include/cpp.h
	$(CC) $(libcpp_CFLAGS) -c cpp.c

main.o: main.c ../include/cpp.h
	$(CC) $(cpp_CFLAGS) -c main.c

clean:
	$(RM) $(libcpp_OBJS) $(cpp_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 libcpp.a $(DESTDIR)$(LIBDIR)/libcpp.a
	$(INSTALL) -m 0755 libcpp.so $(DESTDIR)$(LIBDIR)/libcpp.so
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 cpp $(DESTDIR)$(BINDIR)/cpp

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/libcpp.a
	$(RM) $(DESTDIR)$(LIBDIR)/libcpp.so
	$(RM) $(DESTDIR)$(BINDIR)/cpp

.PHONY: all clean distclean install uninstall
