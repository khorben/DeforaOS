#!/usr/bin/env sh



#variables
[ -z "$ARCH" ] && ARCH=`uname -m`
[ -z "$CVSROOT" ] && CVSROOT=":pserver:anonymous@cvs.defora.org:/Data/CVS"
[ -z "$OS" ] && OS=`uname -s`
#private
DEVNULL="/dev/null"
MODULE="DeforaOS"
SRC="$HOME/build/$OS-$ARCH"
DST="$HOME/destdir/$OS-$ARCH"
#executables
CVS="cvs -q"
MAKE="make"
MKDIR="mkdir -p"
RM="rm -f"

export CVSROOT


#functions
#error
error()
{
	[ ! -z "$1" ] && echo "$1" 1>&2
	$RM -r "$DST"
	$RM -r "$SRC"
	exit 2
}


#main
#configure cvs if necessary
[ ! -f "$HOME/.cvspass" ] && touch "$HOME/.cvspass"
[ ! -f "$HOME/.cvsrc" ] && cat > "$HOME/.cvsrc" << EOF
cvs -q
update -dPA
EOF

#checkout tree
$RM -r "$SRC"
$MKDIR "$SRC"							|| error
cd "$SRC"							|| error
echo ""
echo "Checking out CVS module $MODULE:"
$CVS co "$MODULE" > "$DEVNULL"					|| error
SRC="$SRC/$MODULE"

#create directories
$RM -r "$DST"
$MKDIR "$DST"							|| error

#configuring tree
echo ""
echo "Configuring CVS module $MODULE:"
cd "$SRC"							|| error
$MAKE DESTDIR="$DESTDIR"					|| error

#build
echo ""
echo "Building CVS module $MODULE:"
./build.sh MAKE="$MAKE" DESTDIR="$DST" install			|| error

#cleanup
$RM -r "$SRC"
$RM -r "$DST"
