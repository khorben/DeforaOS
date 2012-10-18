TARGETS	= tcp.so template.so udp.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W -fPIC
CFLAGS	= -Wall -g -O2 -pedantic
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

tcp_OBJS = tcp.o
tcp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
tcp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

tcp.so: $(tcp_OBJS)
	$(CCSHARED) -o tcp.so $(tcp_OBJS) $(tcp_LDFLAGS)

template_OBJS = template.o
template_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
template_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

template.so: $(template_OBJS)
	$(CCSHARED) -o template.so $(template_OBJS) $(template_LDFLAGS)

udp_OBJS = udp.o
udp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
udp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

udp.so: $(udp_OBJS)
	$(CCSHARED) -o udp.so $(udp_OBJS) $(udp_LDFLAGS)

tcp.o: tcp.c
	$(CC) $(tcp_CFLAGS) -c tcp.c

template.o: template.c
	$(CC) $(template_CFLAGS) -c template.c

udp.o: udp.c
	$(CC) $(udp_CFLAGS) -c udp.c

clean:
	$(RM) -- $(tcp_OBJS) $(template_OBJS) $(udp_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/App/transport
	$(INSTALL) -m 0644 -- tcp.so $(DESTDIR)$(LIBDIR)/App/transport/tcp.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/App/transport
	$(INSTALL) -m 0644 -- udp.so $(DESTDIR)$(LIBDIR)/App/transport/udp.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/App/transport/tcp.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/App/transport/udp.so

.PHONY: all clean distclean install uninstall
