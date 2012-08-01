#!/bin/sh
#$Id$
#Copyright (c) 2010-2012 Pierre Pronchery <khorben@defora.org>
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



#variables
PREFIX="/usr/local"
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
	echo "$@" 1>&2
	"$@"
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

	_gettext_po "$package" "$lang"				|| return 1
	$DEBUG $MSGFMT -c -v -o "$lang.mo" "$lang.po"		|| return 1
}


#gettext_po
_gettext_po()
{
	package="$1"
	lang="$2"

	if [ -f "$lang.po" ]; then
		$DEBUG $MSGMERGE -U "$lang.po" "$package.pot"	|| return 1
	else
		$DEBUG $MSGINIT -l "$lang" -o "$lang.po" -i "$package.pot" \
								|| return 1
	fi
}


#gettext_pot
_gettext_pot()
{
	package="$1"

	$DEBUG $XGETTEXT -d "$package" -o "$package.pot" --keyword="_" \
			--keyword="N_" -f "$POTFILES"		|| return 1
}


#main
install=0
uninstall=0
while getopts iuP: name; do
	case "$name" in
		i)
			uninstall=0
			install=1
			;;
		u)
			install=0
			uninstall=1
			;;
		P)
			PREFIX="$2"
			;;
		?)
			_usage
			exit $?
			;;
	esac
done
shift $(($OPTIND - 1))
if [ $# -eq 0 ]; then
	_usage
	exit $?
fi

LOCALEDIR="$PREFIX/share/locale"
while [ $# -gt 0 ]; do
	target="$1"
	lang="${target%%.mo}"
	lang="${lang%%.po}"
	shift

	#uninstall
	if [ "$uninstall" -eq 1 ]; then
		$DEBUG $RM "$LOCALEDIR/$lang/LC_MESSAGES/$PACKAGE.mo" \
								|| exit 2
		continue
	fi

	#install
	if [ "$install" -eq 1 ]; then
		$DEBUG $MKDIR "$LOCALEDIR/$lang/LC_MESSAGES"	|| exit 2
		$DEBUG $INSTALL "$target" \
			"$LOCALEDIR/$lang/LC_MESSAGES/$PACKAGE.mo" \
								|| exit 2
		continue
	fi

	#create
	case "$target" in
		*.mo)
			_gettext_mo "$PACKAGE" "$lang"		|| exit 2
			;;
		*.po)
			_gettext_po "$PACKAGE" "$lang"		|| exit 2
			;;
		*.pot)
			_gettext_pot "${target%%.pot}"		|| exit 2
			;;
		*)
			exit 2
			;;
	esac
done
