TARGETS	= elf.a elf.so
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


clean:
	$(RM) $(elf_OBJS)

distclean: clean
	$(RM) $(TARGETS)
