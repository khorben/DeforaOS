#!/usr/bin/env sh
#$Id$
#Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>



#variables
[ -z "$ARCH" ] && ARCH=`uname -m`
case "$ARCH" in
	i486|i586|i686)
		ARCH="i386"
		;;
	x86_64)
		ARCH="amd64"
		;;
esac
[ -z "$CVSROOT" ] && CVSROOT=":pserver:anonymous@anoncvs.defora.org:/Data/CVS"
[ -z "$OS" ] && OS=`uname -s`
#private
DATE=`date '+%Y%m%d'`
DESTDIR="/var/www/htdocs/download/snapshots"
DEVNULL="/dev/null"
EMAIL="build@lists.defora.org"
FILE="DeforaOS-daily.iso"
KERNEL_VERSION="2.4.37.7"
KERNEL_PATH="/usr/src/linux-$KERNEL_VERSION"
MODULE="DeforaOS"
PREFIX="/usr"
SRC="$HOME/build/$OS-$ARCH"
DST="$HOME/destdir/$OS-$ARCH"

#executables
CP="cp -f"
CONFIGURE="Apps/Devel/src/configure/src/configure -O DeforaOS"
CVS="cvs -q"
MAIL="mail"
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


#deforaos_build
deforaos_build()
{
	#configure cvs if necessary
	[ ! -f "$HOME/.cvspass" ] && touch "$HOME/.cvspass"

	#checkout tree
	$RM -r "$SRC"
	$MKDIR "$SRC"						|| error
	cd "$SRC"						|| error
	echo ""
	echo "Checking out CVS module $MODULE:"
	$CVS co "$MODULE" > "$DEVNULL"				|| error
	SRC="$SRC/$MODULE"

	#create directories
	$RM -r "$DST"
	$MKDIR "$DST"						|| error

	#configuring tree
	echo ""
	echo "Configuring CVS module $MODULE:"
	cd "$SRC"						|| error
	$MAKE DESTDIR="$DST" PREFIX="$PREFIX" bootstrap < "$DEVNULL" \
								|| error

	#create CD-ROM image
	echo ""
	echo "Creating CD-ROM image:"
	./build.sh CONFIGURE="$CONFIGURE" MAKE="$MAKE" \
			DESTDIR="$DST" PREFIX="$PREFIX" \
			IMAGE_TYPE="ramdisk" \
			IMAGE_FILE="initrd.img" IMAGE_SIZE=8192 \
			IMAGE_MODULES="$KERNEL_PATH/modules-ramdisk.tgz" \
			clean image				|| error
	$RM -r "$DST"
	./build.sh CONFIGURE="$CONFIGURE" MAKE="$MAKE" \
			DESTDIR="$DST" PREFIX="$PREFIX" \
			IMAGE_TYPE="iso" \
			IMAGE_FILE="DeforaOS-daily.iso" \
			IMAGE_KERNEL="$KERNEL_PATH/arch/$ARCH/boot/bzImage" \
			IMAGE_MODULES="$KERNEL_PATH/modules.tgz" \
			IMAGE_RAMDISK="initrd.img" \
			KERNEL_ARGS="vga=0x301 rw" \
			clean image				|| error
	$CP "$FILE" "$DESTDIR"					|| error
	echo "http://www.defora.org/download/snapshots/$FILE"

	#cleanup
	$RM -r "$SRC"
	$RM -r "$DST"
}


#main
deforaos_build 2>&1 | $MAIL -s "Daily CVS build Linux $ARCH: $DATE" "$EMAIL"
