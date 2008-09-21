#!/usr/bin/env sh
#$Id$
#Copyright (c) 2008 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS



#variables
CDROM_IMAGE=
CFLAGS=
CPPFLAGS=
DISK_IMAGE=
DISK_SIZE="20480"
LDFLAGS=
DESTDIR=
FLOPPY_IMAGE=
FLOPPY_SIZE="2880"
IMAGE_IMAGE=
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
VENDOR="DeforaOS"

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
	Apps/Unix/src/utils \
	Apps/Unix/src/devel \
	Apps/Unix/src/others"


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
		sparc64)
			MACHINE="sparc"
			;;
	esac
	TARGET="$SYSTEM-$MACHINE"
fi
if [ ! -f "Apps/Devel/src/scripts/targets/$TARGET" ]; then
	echo "$0: warning: $TARGET: Unsupported target" 1>&2
else
	source "Apps/Devel/src/scripts/targets/$TARGET"
fi

#initialize variables
[ -z "$CDROM_IMAGE" ] && CDROM_IMAGE="$VENDOR-cdrom.iso"
[ -z "$DISK_IMAGE" ] && DISK_IMAGE="$VENDOR-disk.img"
[ -z "$FLOPPY_IMAGE" ] && FLOPPY_IMAGE="$VENDOR-floppy.img"
[ -z "$IMAGE_IMAGE" ] && IMAGE_IMAGE="$VENDOR-image.img"
[ -z "$LIBGCC" ] && LIBGCC=`gcc -print-libgcc-file-name`
[ -z "$PREFIX" ] && PREFIX="/usr/local"
[ -z "$CFLAGS" ] && CFLAGS="-Wall -ffreestanding -g"
[ -z "$CPPFLAGS" ] && CPPFLAGS="-nostdinc -I $DESTDIR$PREFIX/include"
[ -z "$LDFLAGS" ] && LDFLAGS="-nostdlib -static $DESTDIR$PREFIX/lib/start.o $DESTDIR$PREFIX/lib/libc.a $LIBGCC"
[ -z "$RAMDISK_IMAGE" ] && RAMDISK_IMAGE="$VENDOR-ramdisk.img"
[ -z "$UID" ] && UID=`id -u`
[ -z "$SUDO" -a "$UID" -ne 0 ] && SUDO="sudo"

#run targets
if [ $# -lt 1 ]; then
	usage
	exit $?
fi
while [ $# -gt 0 ]; do
	case "$1" in
		all)
			echo "$0: Making target $1 on $TARGET" 1>&2
			target "install"			|| exit 2
			;;
		clean|distclean|install|uninstall)
			echo "$0: Making target $1 on $TARGET" 1>&2
			target "$1"				|| exit 2
			;;
		floppy)
			echo "$0: Making target $1 on $TARGET" 1>&2
			[ -z "$DESTDIR" ] && error "DESTDIR needs to be set"
			$MKDIR "$DESTDIR"			|| exit 2
			$DD if="$DEVZERO" of="$DESTDIR/$FLOPPY_IMAGE" \
			       count="$FLOPPY_SIZE"		|| exit 2
			$MKFS "$DESTDIR/$FLOPPY_IMAGE"		|| exit 2
			#FIXME fill floppy image
			;;
		image)
			echo "$0: Making target $1 on $TARGET" 1>&2
			[ -z "$DESTDIR" ] && error "DESTDIR needs to be set"
			$MKDIR "$DESTDIR"			|| exit 2
			target "install"			|| exit 2
			target_image				|| exit 2
			;;
		iso)
			echo "$0: Making target $1 on $TARGET" 1>&2
			[ -z "$DESTDIR" ] && error "DESTDIR needs to be set"
			$MKDIR "$DESTDIR"			|| exit 2
			target "install"			|| exit 2
			target_iso				|| exit 2
			;;
		ramdisk)
			echo "$0: Making target $1 on $TARGET" 1>&2
			[ -z "$DESTDIR" ] && error "DESTDIR needs to be set"
			[ -z "$RAMDISK_IMAGE" ] && error \
				"RAMDISK_IMAGE needs to be set"
			target_ramdisk
			;;
		*)
			echo "build.sh: $1: Unknown target" 1>&2
			usage
			exit $?
			;;
	esac
	shift
done
