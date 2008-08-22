#!/usr/bin/env sh
#$Id$
#Copyright (c) 2008 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS



#variables
CDROM_IMAGE="DeforaOS.iso"
CFLAGS=
CPPFLAGS=
DISK_IMAGE="DeforaOS.img"
DISK_SIZE="20480"
LDFLAGS=
DESTDIR=
FLOPPY_IMAGE="DeforaOS.boot"
FLOPPY_SIZE="2880"
KERNEL=
KERNEL_ARGS=
KERNEL_MODULES=
LIBGCC=
MACHINE=
MOUNTPOINT=
PREFIX=
RAMDISK_IMAGE=
RAMDISK_SIZE="4096"
SYSTEM=
TARGET=

#executables
CAT="cat"
CC=
CP="cp -f"
DD="dd bs=1024"
GZIP="gzip -9"
LD=
LN="ln -sf"
MAKE="make"
MKDIR="mkdir -p"
MKFS=
MKISOFS="mkisofs -R -J -V DeforaOS"
MOUNT=
MV="mv -f"
SUDO=
TUNE2FS=
UMOUNT=

#internals
DEVZERO="/dev/zero"
PROGNAME="$0"
SUBDIRS="System/src/libc \
	Apps/Unix/src/sh \
	Apps/Unix/src/utils"


#functions
#error
error()
{
	echo "build.sh: $1" 1>&2
	exit 2
}


#usage
usage()
{
	echo "Usage: build.sh [option=value...] target..." 1>&2
	echo "Targets:" 1>&2
	echo "  all		build everything" 1>&2
	echo "  clean		remove object files" 1>&2
	echo "  distclean	remove all compiled files" 1>&2
	echo "  floppy	create bootable floppy image" 1>&2
	echo "  install	install everything" 1>&2
	echo "  image		create filesystem image" 1>&2
	echo "  iso		create bootable CD-ROM image" 1>&2
	echo "  ramdisk	create bootable ramdisk image" 1>&2
	echo "  uninstall	uninstall everything" 1>&2
	exit 1
}


#target
target()
{
	_MAKE="$MAKE"
	[ ! -z "$DESTDIR" ] && _MAKE="$_MAKE DESTDIR=\"$DESTDIR\""
	[ ! -z "$PREFIX" ] && _MAKE="$_MAKE PREFIX=\"$PREFIX\""
	[ ! -z "$CC" ] && _MAKE="$_MAKE CC=\"$CC\""
	[ ! -z "$CPPFLAGS" ] && _MAKE="$_MAKE CPPFLAGS=\"$CPPFLAGS\""
	[ ! -z "$CFLAGS" ] && _MAKE="$_MAKE CFLAGS=\"$CFLAGS\""
	[ ! -z "$LD" ] && _MAKE="$_MAKE LD=\"$LD\""
	[ ! -z "$LDFLAGS" ] && _MAKE="$_MAKE LDFLAGS=\"$LDFLAGS\""
	for i in $SUBDIRS; do
		(cd "$i" && eval $_MAKE "$1") || return $?
	done
	return 0
}


#main
#parse options
while [ $# -gt 0 ]; do
	case "$1" in
		*=*)
			VAR=${1%%=*}
			VALUE=${1#*=}
			eval "$VAR='$VALUE'; export $VAR"
			shift
			;;
		*)
			break
			;;
	esac
done

#initialize target
[ -z "$MACHINE" ] && MACHINE=`uname -m`
[ -z "$SYSTEM" ] && SYSTEM=`uname -s`
[ -z "$TARGET" ] && TARGET="$SYSTEM-$MACHINE"
if [ ! -f "Apps/Devel/src/scripts/targets/$TARGET" ]; then
	case "$MACHINE" in
		arm*b|arm*l)
			MACHINE="arm"
			;;
		i?86)
			MACHINE="i386"
			;;
	esac
	TARGET="$SYSTEM-$MACHINE"
fi
if [ ! -f "Apps/Devel/src/scripts/targets/$TARGET" ]; then
	echo "$0: warning: $TARGET: Unsupported target" 1>&2
else
	echo "$0: $TARGET: Loading target" 1>&2
	source "Apps/Devel/src/scripts/targets/$TARGET"
fi

#initialize variables
[ -z "$LIBGCC" ] && LIBGCC=`gcc -print-libgcc-file-name`
[ -z "$PREFIX" ] && PREFIX="/usr/local"
[ -z "$CFLAGS" ] && CFLAGS="-ffreestanding"
[ -z "$CPPFLAGS" ] && CPPFLAGS="-nostdinc -I $DESTDIR$PREFIX/include"
[ -z "$LDFLAGS" ] && LDFLAGS="-nostdlib -static $DESTDIR$PREFIX/lib/start.o $DESTDIR$PREFIX/lib/libc.a $LIBGCC"
[ -z "$SUDO" -a "$UID" -ne 0 ] && SUDO="sudo"

#run targets
if [ $# -lt 1 ]; then
	usage
	exit $?
fi
while [ $# -gt 0 ]; do
	case "$1" in
		all)
			target "install"			|| exit 2
			;;
		clean|distclean|install|uninstall)
			target "$1"				|| exit 2
			;;
		floppy)
			[ -z "$DESTDIR" ] && error "DESTDIR needs to be set"
			$MKDIR "$DESTDIR"			|| exit 2
			$DD if="$DEVZERO" of="$DESTDIR/$FLOPPY_IMAGE" \
			       count="$FLOPPY_SIZE"		|| exit 2
			$MKFS "$DESTDIR/$FLOPPY_IMAGE"		|| exit 2
			#FIXME fill floppy image
			;;
		image)
			[ -z "$DESTDIR" ] && error "DESTDIR needs to be set"
			$MKDIR "$DESTDIR"			|| exit 2
			target "install"			|| exit 2
			target_image				|| exit 2
			;;
		iso)
			[ -z "$DESTDIR" ] && error "DESTDIR needs to be set"
			$MKDIR "$DESTDIR"			|| exit 2
			target "install"			|| exit 2
			target_iso				|| exit 2
			;;
		ramdisk)
			[ -z "$DESTDIR" ] && error "DESTDIR needs to be set"
			[ -z "$MOUNTPOINT" ] && error \
				"MOUNTPOINT needs to be set"
			[ -z "$RAMDISK_IMAGE" ] && error \
				"RAMDISK_IMAGE needs to be set"
			$UMOUNT "$MOUNTPOINT"
			$MKDIR "$MOUNTPOINT"
			$DD if="$DEVZERO" of="$RAMDISK_IMAGE" \
				count="$RAMDISK_SIZE"		|| exit 2
			$MKFS "$RAMDISK_IMAGE"			|| exit 2
			[ ! -z "$TUNE2FS" ] && $TUNE2FS -i 0 "$RAMDISK_IMAGE"
			$MOUNT "$RAMDISK_IMAGE" "$MOUNTPOINT"	|| exit 2
			SUBDIRS="Apps/Unix/src/others/tools" target linuxrc \
				 || exit 2
			for i in "dev" "proc" "mnt/cdrom" "sbin"; do
				$MKDIR "$MOUNTPOINT/$i"		|| exit 2
			done
			$LN "../mnt/cdrom/usr/bin/sh" "$MOUNTPOINT/sbin/init"
			$CP "Apps/Unix/src/others/tools/linuxrc" "$MOUNTPOINT" \
				|| exit 2
			$UMOUNT "$MOUNTPOINT"
			$GZIP "$RAMDISK_IMAGE"			|| exit 2
			$MV "$RAMDISK_IMAGE.gz" "$RAMDISK_IMAGE"|| exit 2
			;;
		*)
			echo "build.sh: $1: Unknown target" 1>&2
			usage
			exit $?
			;;
	esac
	shift
done
