TARGETS	= configure
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGSF= -lutils
CC	= cc
RM	= rm -f
INSTALL	= install


all: $(TARGETS)

configure_OBJS= configure.o
configure: $(configure_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o configure $(configure_OBJS)

configure.o: configure.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c configure.c


clean:
	$(RM) $(configure_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(INSTALL) -m 755 configure $(DESTDIR)$(BINDIR)
