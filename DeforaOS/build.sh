#!/bin/sh
#$Id$
#Copyright (c) 2008 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS



#variables
CC=
CFLAGS=
CPPFLAGS=
LDFLAGS=
DESTDIR=
MAKE="make"
PREFIX=

#internals
PROGNAME="$0"
SUBDIRS="System/src/libc \
	 Apps/Unix/src/devel \
	 Apps/Unix/src/utils \
	 Apps/Unix/src/others"


#functions
#usage
usage()
{
	echo "Usage: build.sh [option=value...] target..." 1>&2
	echo "Targets:" 1>&2
	echo "  all		build everything" 1>&2
	echo "  clean		remove object files" 1>&2
	echo "  distclean	remove all compiled files" 1>&2
	echo "  install	install everything" 1>&2
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
		(cd "$i" && eval $_MAKE "$1") || exit 1
	done
}


#main
#parse options
while [ $# -gt 0 ]; do
	case "$1" in
		*=*)
			VAR=${1%%=*}
			VALUE=${1##*=}
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
		image)
			target "install"
			;;
		*)
			echo "build.sh: $1: Unknown target" 1>&2
			usage
			exit $?
			;;
	esac
	shift
done
