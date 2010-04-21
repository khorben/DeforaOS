#!/bin/sh
#$Id$



#variables
. "../config.sh"
DEBUG="_debug"
INSTALL="install -m 0644"
LOCALEDIR="$PREFIX/share/locale"
MKDIR="mkdir -p"
MSGFMT="msgfmt"
MSGINIT="msginit"
MSGMERGE="msgmerge"
POTFILES="POTFILES"
XGETTEXT="xgettext --force-po"


#functions
#usage
_usage()
{
	echo "Usage: ./gettext.sh target" 1>&2
	return 1
}


#debug
_debug()
{
	echo $@
	$@
}


#gettext_mo
_gettext_mo()
{
	package="$1"
	lang="$2"

	if [ -f "$lang.po" ]; then
		$DEBUG $MSGMERGE -U "$lang.po" "$package.pot"	|| return 1
	else
		$DEBUG $MSGINIT -l "$lang" -o "$lang.po" -i "$package.pot" \
								|| return 1
	fi
	$DEBUG $MSGFMT -c -v -o "$lang.mo" "$lang.po"		|| return 1
}


#gettext_pot
_gettext_pot()
{
	package="$1"

	$DEBUG $XGETTEXT -d "$package" -o "$package.pot" --keyword="_" \
			--keyword="N_" -f "$POTFILES"		|| return 1
}


#main
if [ $# -eq 4 -a "$1" = "-p" -a "$3" = "install" ]; then
	PREFIX="$2"
	LOCALEDIR="$PREFIX/share/locale"
	lang="${4%%.mo}"

	$DEBUG $MKDIR "$LOCALEDIR/$lang/LC_MESSAGES"		|| exit 2
	$DEBUG $INSTALL "$4" "$LOCALEDIR/$lang/LC_MESSAGES/$PACKAGE.mo" \
								|| exit 2
	exit 0
fi
if [ $# -ne 1 ]; then
	_usage
	exit $?
fi
case "$1" in
	*.mo)
		_gettext_mo "$PACKAGE" "${1%%.mo}"		|| exit 2
		;;
	*.pot)
		_gettext_pot "${1%%.pot}"			|| exit 2
		;;
	*)
		exit 2
		;;
esac
exit 0
