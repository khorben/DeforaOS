# make file for "flex" tool

# @(#) $Header$ (LBL)

# Porting considerations:
#
#    For System V Unix machines, add -DUSG to CFLAGS (if it's not
#         automatically defined)
#    For Vax/VMS, add "-DVMS -DUSG" to CFLAGS.
#    For MS-DOS, add "-DMS_DOS -DUSG" to CFLAGS.  Create \tmp if not present.
#         You will also want to rename flex.skel to something with a three
#         character extension, change SKELETON_FILE below appropriately,
#	  See MSDOS.notes for more info.
#    For Amiga, add "-DAMIGA -DUSG" to CFLAGS.
#    For SCO Unix, add "-DSCO_UNIX" to CFLAGS.
#
#    For C compilers which don't know about "void", add -Dvoid=int to CFLAGS.
#
#    If your C compiler is ANSI standard but does not include the <stdlib.h>
#    header file (some installations of gcc have this problem), then add
#    -DDONT_HAVE_STDLIB_H to CFLAGS.
#
# By default, flex will be configured to generate 8-bit scanners only
# if the -8 flag is given.  If you want it to always generate 8-bit
# scanners, add "-DDEFAULT_CSIZE=256" to CFLAGS.  Note that doing
# so will double the size of all uncompressed scanners.
# 
# If on your system you have trouble building flex due to 8-bit
# character problems, remove the -8 from FLEX_FLAGS and the
# "#define FLEX_8_BIT_CHARS" from the beginning of flexdef.h.


# the first time around use "make first_flex"


# Installation targeting.  Files will be installed under the tree rooted
# at DESTDIR.  User commands will be installed in BINDIR, library files
# in LIBDIR (which will be created if necessary), auxiliary files in
# AUXDIR.
DESTDIR =
BINDIR = /usr/bin
LIBDIR = /usr/lib/flex
AUXDIR = /usr/lib/flex

# MAKE = make


SKELETON_FILE = $(DESTDIR)$(AUXDIR)/flex.skel
SKELFLAGS = -DDEFAULT_SKELETON_FILE=\"$(SKELETON_FILE)\"
CFLAGS = $(OPT) -wo -D_MINIX -D_POSIX_SOURCE -DUSG
LDFLAGS = -i

COMPRESSION =
FLEX_FLAGS = -ist8 -Sflex.skel
# which "flex" to use to generate scan.c from scan.l
FLEX = PATH=.:$$PATH flex
# CC = cc

FLEXOBJS = \
	ccl.o \
	dfa.o \
	ecs.o \
	gen.o \
	main.o \
	misc.o \
	nfa.o \
	parse.o \
	scan.o \
	sym.o \
	tblcmp.o \
	yylex.o

FLEX_C_SOURCES = \
	ccl.c \
	dfa.c \
	ecs.c \
	gen.c \
	main.c \
	misc.c \
	nfa.c \
	parse.c \
	scan.c \
	sym.c \
	tblcmp.c \
	yylex.c

FLEX_LIB_OBJS = \
	libmain.o

FLEXLIB = flexlib.a


all : flex

flex : $(FLEXOBJS)
	$(CC) $(CFLAGS) -o flex $(LDFLAGS) $(FLEXOBJS)

first_flex:
	cp initscan.c scan.c
	$(MAKE) $(MFLAGS) flex

parse.h parse.c : parse.y
	$(YACC) -d parse.y
	@mv y.tab.c parse.c
	@mv y.tab.h parse.h

scan.c : scan.l
	$(FLEX) $(FLEX_FLAGS) $(COMPRESSION) scan.l >scan.c

scan.o : scan.c parse.h flexdef.h
	$(CC) $(CFLAGS) -wa -c scan.c

main.o : main.c flexdef.h
	$(CC) $(CFLAGS) -c $(SKELFLAGS) main.c

ccl.o : ccl.c flexdef.h
dfa.o : dfa.c flexdef.h
ecs.o : ecs.c flexdef.h
gen.o : gen.c flexdef.h
misc.o : misc.c flexdef.h
nfa.o : nfa.c flexdef.h
parse.o : parse.c flexdef.h
sym.o : sym.c flexdef.h
tblcmp.o : tblcmp.c flexdef.h
yylex.o : yylex.c flexdef.h

$(FLEXLIB) : $(FLEX_LIB_OBJS)
	aal cr $(FLEXLIB) $(FLEX_LIB_OBJS)

lint : $(FLEX_C_SOURCES)
	lint $(FLEX_C_SOURCES) > flex.lint

distrib :
	mv scan.c initscan.c
	chmod 444 initscan.c
	$(MAKE) $(MFLAGS) clean

install:	\
	$(DESTDIR)$(BINDIR)/flex $(DESTDIR)$(BINDIR)/lex \
	$(SKELETON_FILE)

$(DESTDIR)$(BINDIR)/flex:	flex
	install -S 512k -cs -o bin flex $@

$(DESTDIR)$(BINDIR)/lex:	$(DESTDIR)$(BINDIR)/flex
	install -l $(DESTDIR)$(BINDIR)/flex $@

$(SKELETON_FILE):	flex.skel
	install -d -o bin $(DESTDIR)$(LIBDIR)
	install -c -o bin flex.skel $@

# Not installed yet.  Should be part of the lib tree.
install-lib: $(DESTDIR)$(LIBDIR) $(FLEXLIB)
	install -c -o bin -m 644 $(FLEXLIB) $(DESTDIR)$(LIBDIR)/libfl.a

clean :
	rm -f core errs flex *.o parse.c *.lint parse.h flex.man tags \
		$(FLEXLIB)

tags :
	ctags $(FLEX_C_SOURCES)

vms :	flex.man
	$(MAKE) $(MFLAGS) distrib

test : flex
	./flex $(FLEX_FLAGS) $(COMPRESSION) scan.l | diff scan.c -

bigtest :
	rm -f scan.c ; $(MAKE) COMPRESSION="-C" test
	rm -f scan.c ; $(MAKE) COMPRESSION="-Ce" test
	rm -f scan.c ; $(MAKE) COMPRESSION="-Cm" test
	rm -f scan.c ; $(MAKE) COMPRESSION="-Cfe" test
	rm -f scan.c ; $(MAKE) COMPRESSION="-CFe" test
	rm -f scan.c ; $(MAKE) COMPRESSION="-Cf" test
	rm -f scan.c ; $(MAKE) COMPRESSION="-CF" test
	rm -f scan.c ; $(MAKE)
