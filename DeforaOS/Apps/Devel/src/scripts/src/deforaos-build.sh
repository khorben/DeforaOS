#!/usr/bin/env sh
#$Id$
#Copyright (c) 2008-2012 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS Devel scripts
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, version 3 of the License.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.



#environment
umask 022
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
[ -z "$CVSROOT" ] && CVSROOT=":pserver:anonymous@anoncvs.defora.org:/home/cvs"
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
MKDIR="mkdir -m 0755 -p"
RM="rm -f"
TOUCH="touch"

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
_deforaos_build()
{
	#configure cvs if necessary
	$MKDIR "$HOME"						|| exit 2
	if [ ! -f "$HOME/.cvspass" ]; then
		$TOUCH "$HOME/.cvspass"				|| exit 2
	fi

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
_deforaos_build 2>&1 | $MAIL -s "Daily CVS build Linux $ARCH: $DATE" "$EMAIL"
