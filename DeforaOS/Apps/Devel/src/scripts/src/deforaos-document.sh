#!/usr/bin/env sh
#$Id$
#Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS Unix scripts
#TODO:
#- no longer use a temporary folder



#environment
umask 022
#variables
[ -z "$CVSROOT" ] && CVSROOT=":pserver:anonymous@anoncvs.defora.org:/home/cvs"
#private
DATE=`date '+%Y%m%d'`
DESTDIR="/var/www"
DEVNULL="/dev/null"
EMAIL="webmaster@defora.org"
ROOT=`mktemp -d -p "$HOME/temp" "temp.XXXXXX"`
MODULE="DeforaOS"
SRC="$ROOT/$MODULE"

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
#deforaos_document
deforaos_document()
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
		(cd "$ROOT" && $CVS "-d$CVSROOT" co "$MODULE")	|| exit 2
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
		(cd "$path" && $MAKE install DESTDIR="$DESTDIR" PREFIX="/" > "$DEVNULL" 2> "$DEVNULL")
	done

	#erase temporary data
	$RM -r "$ROOT"
}


#main
[ -z "$ROOT" ] && exit 2
deforaos_document 2>&1 | $MAIL -s "Daily CVS documentation: $DATE" "$EMAIL"
