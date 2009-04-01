#!/usr/bin/env sh
#$Id$
#Copyright (c) 2009 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS



#variables
CFLAGS=
CPPFLAGS=
LDFLAGS=
DESTDIR=
HOST=
IMAGE_FILE=
IMAGE_TYPE=
MACHINE=
PREFIX=
SYSTEM=
TARGET=
VENDOR="DeforaOS"

#executables
CC=
DD="dd bs=1024"
INSTALL="install"
LD=
LN="ln -f"
MAKE="make"
MKDIR="mkdir -p"
MKNOD="mknod"
MV="mv -f"
RMDIR="rmdir -p"

#internals
DEVNULL="/dev/null"
DEVZERO="/dev/zero"
PROGNAME="$0"
SUBDIRS="System/src/libc \
	Apps/Unix/src/sh \
	Apps/Unix/src/utils \
	Apps/Unix/src/devel \
	Apps/Unix/src/others"


#functions
#check
check()
{
	USAGE="$1"
	EMPTY=

	shift
	for i in $@; do
		VAR=`eval echo "\\\$\$i"`
		[ -z "$VAR" ] && EMPTY="$EMPTY $i"
	done
	[ -z "$EMPTY" ] && return
	USAGE=`echo -e "$USAGE\n\nError:$EMPTY need to be set"`
	usage "$USAGE"
	exit 2
}


#error
error()
{
	echo "build.sh: $1" 1>&2
	exit 2
}


#usage
usage()
{
	echo "Usage: build.sh [option=value...] target..."
	echo "Targets:"
	echo "  all		Build everything"
	echo "  clean		Remove object files"
	echo "  distclean	Remove all compiled files"
	echo "  install	Install everything"
	echo "  image		Create a specific image"
	echo "  uninstall	Uninstall everything"
	if [ ! -z "$1" ]; then
		echo
		echo "$1"
	fi
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
		x86_64)
			MACHINE="amd64"
			;;
	esac
	TARGET="$SYSTEM-$MACHINE"
fi
if [ ! -f "Apps/Devel/src/scripts/targets/$TARGET" ]; then
	echo "$0: warning: $TARGET: Unsupported target" 1>&2
else
	. "Apps/Devel/src/scripts/targets/$TARGET"
fi

#initialize variables
[ -z "$CC" ] && CC="cc"
[ -z "$IMAGE_TYPE" ] && IMAGE_TYPE="image"
[ -z "$IMAGE_FILE" ] && IMAGE_FILE="$VENDOR-$IMAGE_TYPE.img"
[ -z "$DESTDIR" ] && DESTDIR="$PWD/destdir-$TARGET"
[ -z "$PREFIX" ] && PREFIX="/usr/local"
[ -z "$CC" ] && CC="cc"
[ -z "$CPPFLAGS" ] && CPPFLAGS="-nostdinc -isystem $DESTDIR$PREFIX/include"
[ -z "$CFLAGS" ] && CFLAGS="-Wall -ffreestanding -g"
[ -z "$LDFLAGS" ] && LDFLAGS="-nostdlib -L $DESTDIR$PREFIX/lib -Wl,-rpath-link,$PREFIX/lib -Wl,-rpath,$PREFIX/lib -l c `$CC -print-libgcc-file-name` $DESTDIR$PREFIX/lib/start.o"
[ -z "$UID" ] && UID=`id -u`
[ -z "$SUDO" -a "$UID" -ne 0 ] && SUDO="sudo"

#run targets
if [ $# -lt 1 ]; then
	usage
	exit $?
fi
while [ $# -gt 0 ]; do
	case "$1" in
		all|install)
			echo "$PROGNAME: Making target $1 on $TARGET" 1>&2
			target "install"			|| exit 2
			;;
		clean|distclean|uninstall)
			echo "$PROGNAME: Making target $1 on $TARGET" 1>&2
			target "$1"				|| exit 2
			;;
		image)
			echo "$PROGNAME: Making target $1 on $TARGET" 1>&2
			target_image				|| exit 2
			;;
		*)
			echo "build.sh: $1: Unknown target" 1>&2
			usage
			exit $?
			;;
	esac
	shift
done
