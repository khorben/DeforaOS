#!/usr/bin/env sh
#$Id$



#variables
[ -z "$ARCH" ] && ARCH=`uname -m`
[ -z "$CVSROOT" ] && CVSROOT=":pserver:anonymous@anoncvs.defora.org:/Data/CVS"
[ -z "$OS" ] && OS=`uname -s`
#private
DESTDIR="/var/www/htdocs/download/snapshots"
DEVNULL="/dev/null"
MODULE="DeforaOS"
PREFIX="/usr"
SRC="$HOME/build/$OS-$ARCH"
DST="$HOME/destdir/$OS-$ARCH"

#executables
CP="cp -f"
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
#check variables
case "$ARCH" in
	i*86)
		ARCH="i386"
		;;
	x86_64)
		ARCH="amd64"
		;;
esac

#configure cvs if necessary
[ ! -f "$HOME/.cvspass" ] && touch "$HOME/.cvspass"

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
./build.sh MAKE="$MAKE" DESTDIR="$DST" PREFIX="$PREFIX" install	|| error

#create CD-ROM image
echo ""
echo "Creating CD-ROM image:"
./build.sh MAKE="$MAKE" DESTDIR="$DST" PREFIX="$PREFIX" \
		IMAGE_TYPE="ramdisk" IMAGE_FILE="initrd.img" IMAGE_SIZE=8192 \
		IMAGE_MODULES="/usr/src/linux-2.4.37/modules-ramdisk.tgz" \
		image						|| error
$RM -r "$DST"
./build.sh MAKE="$MAKE" DESTDIR="$DST" PREFIX="$PREFIX" \
		IMAGE_TYPE="iso" \
		IMAGE_FILE="DeforaOS-daily.iso" \
		IMAGE_KERNEL="/usr/src/linux-2.4.37/arch/i386/boot/bzImage" \
		IMAGE_MODULES="/usr/src/linux-2.4.37/modules.tgz" \
		IMAGE_RAMDISK="initrd.img" \
		KERNEL_ARGS="vga=0x301 rw" \
		image						|| error
$CP "DeforaOS-daily.iso" "$DESTDIR"				|| error
echo "http://www.defora.org/download/snapshots/DeforaOS-daily.tar.gz"

#cleanup
$RM -r "$SRC"
$RM -r "$DST"
