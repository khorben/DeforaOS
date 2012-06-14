#!/usr/bin/env sh
#$Id$
#Copyright (c) 2008-2012 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS Unix scripts



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
RM="rm -f"
TAR="tar"
TOUCH="touch"


#functions
#deforaos_update
deforaos_update()
{
	#configure cvs if necessary
	[ ! -f "$HOME/.cvspass" ] && $TOUCH "$HOME/.cvspass"

	#checkout tree if necessary
	if [ ! -d "$SRC" ]; then
		echo ""
		echo "Checking out CVS module $MODULE:"
		$CVS "-d$CVSROOT" co "$MODULE" || exit 1
	fi

	#update tree
	echo ""
	echo "Updating CVS module $MODULE:"
	(cd "$SRC" && $CVS update -dPA)				|| return 2

	#document tree
	echo ""
	echo "Documenting CVS module $MODULE:"
	$FIND "$SRC/System" "$SRC/Apps" -name "doc" | while read path; do
		[ -x "$path/docbook.sh" ] || continue
		for i in $path/*.xml; do
			(cd "$path" && $MAKE install DESTDIR="$DESTDIR" PREFIX="/")
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
