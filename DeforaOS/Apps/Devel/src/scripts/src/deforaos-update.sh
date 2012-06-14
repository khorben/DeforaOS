#!/usr/bin/env sh
#$Id$
#Copyright (c) 2008-2012 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS Unix scripts



#environment
umask 022
#variables
[ -z "$CVSROOT" ] && CVSROOT=":pserver:anonymous@anoncvs.defora.org:/home/cvs"
#private
DATE=`date '+%Y%m%d'`
DESTDIR="/var/www"
DEVNULL="/dev/null"
EMAIL="devel@lists.defora.org"
MODULE="DeforaOS"
SRC="$HOME/$MODULE"

#executables
CVS="cvs -q"
FIND="find"
LN="ln -f"
MAIL="mail"
MAKE="make"
MKDIR="mkdir -p"
RM="rm -f"
TAR="tar"
TOUCH="touch"


#functions
#deforaos_update
deforaos_update()
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

	#document tree
	echo ""
	echo "Documenting CVS module $MODULE:"
	$FIND "$SRC/System" "$SRC/Apps" -name "doc" | while read path; do
		[ -x "$path/gtkdoc.sh" ] || continue
		for i in $path/*.xml; do
			(cd "$path" && $MAKE install DESTDIR="$DESTDIR" PREFIX="/") 2> "$DEVNULL"
		done
	done

	#make archive
	echo ""
	echo "Archiving CVS module $MODULE:"
	for i in *; do
		echo "DeforaOS-$DATE/$i"
	done | ($LN -s . "DeforaOS-$DATE" \
			&& xargs $TAR -czf "$DESTDIR/htdocs/download/snapshots/DeforaOS-daily.tar.gz")
	$RM "DeforaOS-$DATE"
	echo "http://www.defora.org/download/snapshots/DeforaOS-daily.tar.gz"
}


#main
deforaos_update 2>&1 | $MAIL -s "Daily CVS update: $DATE" "$EMAIL"
