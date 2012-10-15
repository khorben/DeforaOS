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
ARCH="$(uname -m)"
case "$ARCH" in
	i486|i586|i686)
		ARCH="i386"
		;;
	x86_64)
		ARCH="amd64"
		;;
esac
CVSROOT=":pserver:anonymous@anoncvs.defora.org:/home/cvs"
DATE="$(date '+%Y%m%d')"
DESTDIR="/var/www/htdocs/download/snapshots"
DEVNULL="/dev/null"
EMAIL="build@lists.defora.org"
FILE="DeforaOS-daily.iso"
HOMEPAGE="http://www.defora.org"
KERNEL_VERSION="2.4.37.7"
KERNEL_PATH="/usr/src/linux-$KERNEL_VERSION"
MODULE="DeforaOS"
PREFIX="/usr"
SYSTEM="$(uname -s)"
SRC="$HOME/build/$SYSTEM-$ARCH"
DST="$HOME/destdir/$SYSTEM-$ARCH"

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
_error()
{
	[ ! -z "$1" ] && echo "$1" 1>&2
	$RM -r -- "$DST"
	$RM -r -- "$SRC"
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
	$RM -r -- "$SRC"
	$MKDIR -- "$SRC"					|| _error
	cd "$SRC"						|| _error
	echo ""
	echo "Checking out CVS module $MODULE:"
	$CVS co "$MODULE" > "$DEVNULL"				|| _error
	SRC="$SRC/$MODULE"

	#create directories
	$RM -r -- "$DST"
	$MKDIR -- "$DST"					|| _error

	#configuring tree
	echo ""
	echo "Configuring CVS module $MODULE:"
	cd "$SRC"						|| _error
	$MAKE DESTDIR="$DST" PREFIX="$PREFIX" bootstrap < "$DEVNULL" \
								|| _error

	#create CD-ROM image
	echo ""
	echo "Creating CD-ROM image:"
	./build.sh -O CONFIGURE="$CONFIGURE" -O MAKE="$MAKE" \
			-O DESTDIR="$DST" -O PREFIX="$PREFIX" \
			-O IMAGE_TYPE="ramdisk" \
			-O IMAGE_FILE="initrd.img" -O IMAGE_SIZE="8192" \
			-O IMAGE_MODULES="$KERNEL_PATH/modules-ramdisk.tgz" \
			clean image				|| _error
	$RM -r -- "$DST"
	./build.sh -O CONFIGURE="$CONFIGURE" -O MAKE="$MAKE" \
			-O DESTDIR="$DST" -O PREFIX="$PREFIX" \
			-O IMAGE_TYPE="iso" \
			-O IMAGE_FILE="DeforaOS-daily.iso" \
			-O IMAGE_KERNEL="$KERNEL_PATH/arch/$ARCH/boot/bzImage" \
			-O IMAGE_MODULES="$KERNEL_PATH/modules.tgz" \
			-O IMAGE_RAMDISK="initrd.img" \
			-O KERNEL_ARGS="vga=0x301 rw" \
			clean image				|| _error
	$CP -- "$FILE" "$DESTDIR"				|| _error
	echo "$HOMEPAGE/download/snapshots/$FILE"

	#cleanup
	$RM -r -- "$SRC"
	$RM -r -- "$DST"
}


#usage
_usage()
{
	echo "Usage: deforaos-build.sh [-O name=value...]" 1>&2
	return 1
}


#main
#parse options
while getopts "O:" name; do
	case "$name" in
		O)
			export "${OPTARG%%=*}"="${OPTARG#*=}"
			;;
		*)
			_usage
			exit $?
			;;
	esac
done
shift $((OPTIND - 1))
if [ $# -ne 0 ]; then
	_usage
	exit $?
fi

_deforaos_build 2>&1 | $MAIL -s "Daily CVS build $SYSTEM $ARCH: $DATE" "$EMAIL"
