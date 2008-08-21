#!/usr/bin/env sh



#variables
CVSROOT=":pserver:anonymous@cvs.defora.org:/Data/CVS"
DATE=`date '+%Y%m%d'`
DESTDIR="/var/www/htdocs/download/snapshots"
MODULE="DeforaOS"
SRC="$HOME/$MODULE"

#executables
CVS="cvs"
LN="ln -f"
RM="rm -f"


#functions
#main
#configure cvs if necessary
[ ! -f "$HOME/.cvspass" ] && touch "$HOME/.cvspass"
[ ! -f "$HOME/.cvsrc" ] && cat > "$HOME/.cvsrc" << EOF
cvs -q
update -dPA
EOF

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
$CVS update

#make archive
echo ""
echo "Archiving CVS module $MODULE:"
for i in *; do
	echo "DeforaOS-$DATE/$i"
done | ($LN -s . "DeforaOS-$DATE" \
		&& xargs tar -czf "$DESTDIR/DeforaOS-daily.tar.gz")
$RM "DeforaOS-$DATE"
echo "http://www.defora.org/download/snapshots/DeforaOS-daily.tar.gz"
