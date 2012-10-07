#!/usr/bin/env sh
#$Id$
#Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
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
#TODO:
#- no longer use a temporary folder



#environment
umask 022
#variables
[ -z "$CVSROOT" ] && CVSROOT=":pserver:anonymous@anoncvs.defora.org:/home/cvs"
DATE=`date '+%Y%m%d'`
DESTDIR="/var/www"
DEVNULL="/dev/null"
EMAIL="webmaster@defora.org"
ROOT=$(mktemp -d -p "$HOME" "temp.XXXXXX")
MODULE="DeforaOS"
SRC="$ROOT/$MODULE"

#executables
CVS="cvs -q"
FIND="find"
INSTALL="install -m 0644"
LN="ln -f"
MAIL="mail"
MAKE="make"
MKDIR="mkdir -m 0755 -p"
RM="rm -f"
TAR="tar"
TOUCH="touch"


#functions
#deforaos_document
_deforaos_document()
{
	#configure cvs if necessary
	$MKDIR -- "$HOME"					|| exit 2
	if [ ! -f "$HOME/.cvspass" ]; then
		$TOUCH "$HOME/.cvspass"				|| exit 2
	fi

	#checkout tree if necessary
	if [ ! -d "$SRC" ]; then
		echo ""
		echo "Checking out CVS module $MODULE:"
		(cd "$ROOT" && $CVS "-d$CVSROOT" co "$MODULE") > "$DEVNULL" \
								|| exit 2
	fi

	#document tree
	echo ""
	echo "Documenting CVS module $MODULE:"

	#manual pages
	echo ""
	echo " * manual pages"
	(cd "$SRC/Data/Documentation/DeforaOS Manual Pages" &&
		$MAKE &&
		$MKDIR -- "$DESTDIR/htdocs/doc/manual" &&
		$FIND doc/manual -name "*.html" -exec \
		$INSTALL -- {} "$DESTDIR/htdocs/{}" \;)

	#generic documentation
	echo ""
	echo " * generic documentation"
	$FIND "$SRC/System" "$SRC/Apps" -name "doc" | while read path; do
		[ -x "$path/docbook.sh" -o -x "$path/gtkdoc.sh" ] || continue
		(cd "$path" && $MAKE DESTDIR="$DESTDIR" PREFIX="/" \
				install > "$DEVNULL")
	done

	#erase temporary data
	$RM -r "$ROOT"
}


#usage
_usage()
{
	echo "Usage: deforaos-document.sh [-O name=value...]" 1>&2
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
[ -n "$ROOT" ] || exit 2
_deforaos_document 2>&1 | $MAIL -s "Daily CVS documentation: $DATE" "$EMAIL"
