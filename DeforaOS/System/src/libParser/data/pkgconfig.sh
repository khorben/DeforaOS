#!/bin/sh
#$Id$



#variables
. "../config.sh"
DEBUG="_debug"
INSTALL="install -m 0644"
MKDIR="mkdir -p"
RM="rm -f"
SED="sed"


#functions
#debug
_debug()
{
	echo $@
	$@
}


#usage
_usage()
{
	echo "Usage: pkgconfig.sh target" 1>&2
	echo "       pkgconfig.sh -p prefix install target" 1>&2
	echo "       pkgconfig.sh -p prefix uninstall target" 1>&2
	return 1
}


#main
if [ $# -eq 4 -a "$1" = "-p" ]; then
	PREFIX="$2"
	PKGCONFIG="$PREFIX/lib/pkgconfig"

	if [ "$3" = "install" ]; then
		$DEBUG $MKDIR "$PKGCONFIG"			|| exit 2
		$DEBUG $INSTALL "$4" "$PKGCONFIG/$4"		|| exit 2
		exit 0
	elif [ "$3" = "uninstall" ]; then
		$DEBUG $RM "$PKGCONFIG/$4"			|| exit 2
		exit 0
	else
		echo "pkgconfig.sh: $3: Unknown operation" 1>&2
	fi
fi
if [ $# -ne 1 ]; then
	_usage
	exit $?
fi

$SED -e "s,PREFIX,$PREFIX," -e "s,VERSION,$VERSION," "$1.in" > "$1"
