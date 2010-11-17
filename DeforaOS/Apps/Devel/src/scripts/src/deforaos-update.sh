#!/usr/bin/env sh
#$Id$
#Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>



#variables
[ -z "$CVSROOT" ] && CVSROOT=":pserver:anonymous@anoncvs.defora.org:/Data/CVS"
#private
DATE=`date '+%Y%m%d'`
DESTDIR="/var/www/htdocs/download/snapshots"
EMAIL="devel@lists.defora.org"
MODULE="DeforaOS"
SRC="$HOME/$MODULE"

#executables
CVS="cvs -q"
LN="ln -f"
MAIL="mail"
RM="rm -f"


#functions
#deforaos_update
deforaos_update()
{
	#configure cvs if necessary
	[ ! -f "$HOME/.cvspass" ] && touch "$HOME/.cvspass"

	#checkout tree if necessary
	if [ ! -d "$SRC" ]; then
		echo ""
		echo "Checking out CVS module $MODULE:"
		$CVS "-d$CVSROOT" co "$MODULE" || exit 1
	fi

	#update tree
	echo ""
	echo "Updating CVS module $MODULE:"
	cd "$SRC" || exit 1
	$CVS update -dPA

	#make archive
	echo ""
	echo "Archiving CVS module $MODULE:"
	for i in *; do
		echo "DeforaOS-$DATE/$i"
	done | ($LN -s . "DeforaOS-$DATE" \
			&& xargs tar -czf "$DESTDIR/DeforaOS-daily.tar.gz")
	$RM "DeforaOS-$DATE"
	echo "http://www.defora.org/download/snapshots/DeforaOS-daily.tar.gz"
}


#main
deforaos_update 2>&1 | $MAIL -s "Daily CVS update: $DATE" "$EMAIL"
