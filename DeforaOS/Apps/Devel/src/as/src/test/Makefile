TARGETS	= i386.o i386_real.o i486.o java.o
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
AS	= ../as
ASFLAGS	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

i386.o_OBJS = i386.o
i386.o_ASFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(ASFLAGS)

i386_real.o_OBJS = i386_real.o
i386_real.o_ASFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(ASFLAGS) -a i386_real -f flat

i486.o_OBJS = i486.o
i486.o_ASFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(ASFLAGS)

java.o_OBJS = java.o
java.o_ASFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(ASFLAGS) -a java -f java

i386.o: i386.S ../as
	$(AS) $(i386.o_ASFLAGS) -o i386.o i386.S

i386_real.o: i386_real.S ../as
	$(AS) $(i386_real.o_ASFLAGS) -o i386_real.o i386_real.S

i486.o: i486.S ../as
	$(AS) $(i486.o_ASFLAGS) -o i486.o i486.S

java.o: java.S ../as
	$(AS) $(java.o_ASFLAGS) -o java.o java.S

clean:
	$(RM) -- $(i386.o_OBJS) $(i386_real.o_OBJS) $(i486.o_OBJS) $(java.o_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
