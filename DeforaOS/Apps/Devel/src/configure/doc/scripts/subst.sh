#!/bin/sh
#$Id$
#Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
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



#variables
PREFIX="/usr/local"
[ -f "../config.sh" ] && . "../config.sh"
CHMOD="chmod"
DEBUG="_debug"
DEVNULL="/dev/null"
INSTALL="install"
MKDIR="mkdir -m 0755 -p"
RM="rm -f"
SED="sed"


#functions
#debug
_debug()
{
	echo "$@" 1>&2
	"$@"
}


#usage
_usage()
{
	echo "Usage: subst.sh [-i|-u][-P prefix] target" 1>&2
	return 1
}


#main
install=0
uninstall=0
while getopts iuP: name; do
	case $name in
		i)
			uninstall=0
			install=1
			;;
		u)
			install=0
			uninstall=1
			;;
		P)
			PREFIX="$2"
			;;
		?)
			_usage
			exit $?
			;;
	esac
done
shift $(($OPTIND - 1))
if [ $# -eq 0 ]; then
	_usage
	exit $?
fi

while [ $# -gt 0 ]; do
	target="$1"
	shift

	#uninstall
	if [ "$uninstall" -eq 1 ]; then
		$DEBUG $RM -- "$PREFIX/$target"			|| exit 2
		continue
	fi

	#install
	if [ "$install" -eq 1 ]; then
		$DEBUG $MKDIR -- "$PREFIX"			|| exit 2
		mode="-m 0644"
		[ -x "$target" ] && mode="-m 0755"
		$DEBUG $INSTALL $mode "$target" "$PREFIX/$target" \
								|| exit 2
		continue
	fi

	#create
	$DEBUG $SED -e "s,@PREFIX@,$PREFIX," \
		-e "s,@VERSION@,$VERSION," "$target.in" > "$target"
	if [ $? -ne 0 ]; then
		$RM -- "$target" 2> "$DEVNULL"
		exit 2
	elif [ -x "$target.in" ]; then
		$DEBUG $CHMOD 0755 "$target"
	fi
done
