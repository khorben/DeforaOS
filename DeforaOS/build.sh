#!/usr/bin/env sh
#$Id$
#Copyright (c) 2008 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS



#variables
CAT="cat"
CC=
CDROM_IMAGE="DeforaOS.iso"
CFLAGS=
CP="cp -f"
CPPFLAGS=
DISK_IMAGE="DeforaOS.img"
DISK_SIZE="20000"
DD="dd bs=1024"
LDFLAGS=
DESTDIR=
FLOPPY_IMAGE="DeforaOS.boot"
FLOPPY_SIZE="2880"
KERNEL=
KERNEL_ARGS=
KERNEL_MODULES=
MAKE="make"
MKDIR="mkdir -p"
MKFS=
MKISOFS="mkisofs -R -J -V DeforaOS"
MOUNT=
PREFIX=
SUDO=
UMOUNT=

#internals
DEVZERO="/dev/zero"
PROGNAME="$0"
SUBDIRS="System/src/libc \
	Apps/Unix/src/sh \
	Apps/Unix/src/utils"
SYSTEM=`uname -s`


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
	[ ! -z "$LDFLAGS" ] && _MAKE="$_MAKE LDFLAGS=\"$LDFLAGS\""
	for i in $SUBDIRS; do
		(cd "$i" && eval $_MAKE "$1") || exit 2
	done
}


#platform specific
#netbsd
netbsd_mount()
{
	$SUDO vnconfig -c vnd0 "$1" &&
	$SUDO mount /dev/vnd0a "$2"
}


netbsd_umount()
{
	$SUDO umount "$1" &&
	$SUDO vnconfig -u vnd0
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

#initialize variables
[ -z "$CPPFLAGS" ] && CPPFLAGS="-nostdinc -I $DESTDIR$PREFIX/include"
[ -z "$CFLAGS" ] && CFLAGS="-fno-builtin"
[ -z "$LDFLAGS" ] && LDFLAGS="-nostdlib -L $DESTDIR$PREFIX/lib $DESTDIR$PREFIX/lib/start.o $DESTDIR$PREFIX/lib/libc.a"
#platform specific
case "$SYSTEM" in
	NetBSD)
		[ -z "$KERNEL" ] && KERNEL="/netbsd"
		[ -z "$MKFS" ] && MKFS="newfs -F"
		[ -z "$MOUNT" ] && MOUNT="netbsd_mount"
		[ -z "$UMOUNT" ] && UMOUNT="netbsd_umount"
		;;
	*|Linux)
		[ -z "$KERNEL" ] && KERNEL="/vmlinuz"
		[ -z "$MKFS" ] && MKFS="mke2fs -F"
		[ -z "$MOUNT" ] && MOUNT="$SUDO mount -o loop"
		[ -z "$UMOUNT" ] && UMOUNT="$SUDO umount"
		;;
esac

#run targets
if [ $# -lt 1 ]; then
	usage
	exit $?
fi
while [ $# -gt 0 ]; do
	case "$1" in
		all)
			target "install"
			;;
		clean|distclean|install|uninstall)
			target "$1"
			;;
		floppy)
			$MKDIR "$DESTDIR" &&
			$DD if="$DEVZERO" of="$DESTDIR/$FLOPPY_IMAGE" \
			       count="$FLOPPY_SIZE" &&
			$MKFS "$DESTDIR/$FLOPPY_IMAGE"
			#FIXME fill floppy image
			;;
		image)
			[ -z "$DESTDIR" ] && error "DESTDIR needs to be set"
			$UMOUNT "$DESTDIR"
			$MKDIR "$DESTDIR"
			$DD if="$DEVZERO" of="$DISK_IMAGE" count="$DISK_SIZE" &&
			$MKFS "$DISK_IMAGE" &&
			$MOUNT "$DISK_IMAGE" "$DESTDIR" &&
			target "install"
			$UMOUNT "$DESTDIR"
			;;
		iso)
			[ -z "$DESTDIR" ] && error "DESTDIR needs to be set"
			$MKDIR "$DESTDIR" &&
			target "install" &&
			$MKDIR "$DESTDIR/boot/grub" &&
			$CP "/usr/lib/grub/i386-pc/stage2_eltorito" \
				"$DESTDIR/boot/grub" &&
			$CP "$KERNEL" "$DESTDIR/boot/uKernel"
			$CAT > "$DESTDIR/boot/grub/menu.lst" << EOF
default 0
timeout 10

title DeforaOS
kernel /boot/uKernel $KERNEL_ARGS
EOF
			[ ! -z "$KERNEL_MODULES" ] && cat "$KERNEL_MODULES" | \
				(cd "$DESTDIR" && tar xzf -)
			$MKISOFS -b "boot/grub/stage2_eltorito" -no-emul-boot \
				-boot-load-size 4 -boot-info-table \
				-o "$CDROM_IMAGE" "$DESTDIR"
			;;
		*)
			echo "build.sh: $1: Unknown target" 1>&2
			usage
			exit $?
			;;
	esac
	shift
done
