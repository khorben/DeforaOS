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
DEBUG="_debug"
SQLITE2="sqlite"
SQLITE3="sqlite3"


#functions
#debug
_debug()
{
	echo "$@" 1>&2
	"$@"
	#ignore errors when the command is not available
	[ $? -eq 127 ]						&& return 0
}


#usage
_usage()
{
	echo "Usage: database.sh [-P prefix] target" 1>&2
	return 1;
}


#main
while getopts "P:" "name"; do
	case "$name" in
		P)
			#XXX ignored for compatibility
			;;
		?)
			_usage
			exit $?
			;;
	esac
done
shift $((OPTIND - 1))
if [ $# -ne 1 ]; then
	_usage
	exit $?
fi
target="$1"

case "$target" in
	sqlite.db)
		echo .read "../doc/sql/sqlite.sql" | $DEBUG $SQLITE2 "$target"
		#XXX avoid this work-around
		if [ $? -eq 1 ]; then
			echo "database.sh: $target: Error 1 (ignored)" 1>&2
		fi
		;;
	sqlite.db3)
		echo .read "../doc/sql/sqlite.sql" | $DEBUG $SQLITE3 "$target"
		#XXX avoid this work-around
		if [ $? -eq 1 ]; then
			echo "database.sh: $target: Error 1 (ignored)" 1>&2
		fi
		;;
esac
