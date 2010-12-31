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
RM="rm -f"
POTFILES="POTFILES"
XGETTEXT="xgettext --force-po"


#functions
#debug
_debug()
{
	echo $@
	$@
}


#usage
_usage()
{
	echo "Usage: gettext.sh [-i|-u][-P prefix] <target>" 1>&2
	return 1
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
args=`getopt iuP: $*`
if [ $? -ne 0 ]; then
	_usage
	exit $?
fi
set -- $args
install=0
uninstall=0
while [ $# -gt 0 ]; do
	case "$1" in
		-i)
			install=1
			;;
		-u)
			uninstall=1
			;;
		-P)
			PREFIX="$2"
			shift
			;;
		--)
			shift
			break
			;;
	esac
	shift
done

LOCALEDIR="$PREFIX/share/locale"
while [ $# -gt 0 ]; do
	target="$1"
	lang="${target%%.mo}"
	shift

	#uninstall
	if [ "$uninstall" -eq 1 ]; then
		$DEBUG $RM "$LOCALEDIR/$lang/LC_MESSAGES/$PACKAGE.mo" \
								|| exit 2
		continue
	fi

	#create
	case "$target" in
		*.mo)
			_gettext_mo "$PACKAGE" "$lang"		|| exit 2
			;;
		*.pot)
			_gettext_pot "${target%%.pot}"		|| exit 2
			;;
		*)
			exit 2
			;;
	esac

	#install
	if [ "$install" -eq 1 ]; then
		$DEBUG $MKDIR "$LOCALEDIR/$lang/LC_MESSAGES"	|| exit 2
		$DEBUG $INSTALL "$target" \
			"$LOCALEDIR/$lang/LC_MESSAGES/$PACKAGE.mo" \
								|| exit 2
	fi
done
