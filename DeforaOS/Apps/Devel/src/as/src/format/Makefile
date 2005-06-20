TARGETS	= elf.a elf.so flat.a flat.so
CFLAGSF	= -W -Wall -ansi
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

elf_OBJS= elf.o
elf_CFLAGS=$(CFLAGSF) $(CFLAGS)
elf.a: $(elf_OBJS)
	$(AR) elf.a $(elf_OBJS)
	$(RANLIB) elf.a

elf.so: $(elf_OBJS)
	$(LD) -o elf.so $(elf_OBJS)

elf.o: elf.c
	$(CC) $(elf_CFLAGS) -c elf.c

flat_OBJS= flat.o
flat_CFLAGS=$(CFLAGSF) $(CFLAGS)
flat.a: $(flat_OBJS)
	$(AR) flat.a $(flat_OBJS)
	$(RANLIB) flat.a

flat.so: $(flat_OBJS)
	$(LD) -o flat.so $(flat_OBJS)

flat.o: flat.c
	$(CC) $(flat_CFLAGS) -c flat.c


clean:
	$(RM) $(elf_OBJS) $(flat_OBJS)

distclean: clean
	$(RM) $(TARGETS)
