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
[ -z "$CVSROOT" ] && CVSROOT=":pserver:anonymous@anoncvs.defora.org:/home/cvs"
#private
DATE=`date '+%Y%m%d'`
DESTDIR="/var/www"
EMAIL="devel@lists.defora.org"
MODULE="DeforaOS"
SRC="$HOME/$MODULE"

#executables
CVS="cvs -q"
LN="ln -f"
MAIL="mail"
MAKE="make"
MKDIR="mkdir -m 0755 -p"
RM="rm -f"
TAR="tar"
TOUCH="touch"
XARGS="xargs"


#functions
#deforaos_update
_deforaos_update()
{
	#configure cvs if necessary
	$MKDIR "$HOME"						|| exit 2
	if [ ! -f "$HOME/.cvspass" ]; then
		$TOUCH "$HOME/.cvspass"				|| exit 2
	fi

	#checkout tree if necessary
	if [ ! -d "$SRC" ]; then
		echo ""
		echo "Checking out CVS module $MODULE:"
		(cd "$HOME" && $CVS "-d$CVSROOT" co "$MODULE")	|| exit 2
	fi

	#update tree
	echo ""
	echo "Updating CVS module $MODULE:"
	(cd "$SRC" && $CVS update -dPA)				|| exit 2

	#make archive
	echo ""
	echo "Archiving CVS module $MODULE:"
	for i in *; do
		echo "DeforaOS-$DATE/$i"
	done | ($LN -s . "DeforaOS-$DATE" \
			&& $XARGS $TAR -czf "$DESTDIR/htdocs/download/snapshots/DeforaOS-daily.tar.gz")
	$RM "DeforaOS-$DATE"
	echo "http://www.defora.org/download/snapshots/DeforaOS-daily.tar.gz"
}


#main
_deforaos_update 2>&1 | $MAIL -s "Daily CVS update: $DATE" "$EMAIL"
