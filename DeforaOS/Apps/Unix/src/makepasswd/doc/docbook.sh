#!/bin/sh
#$Id$
#Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
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
MKDIR="mkdir -m 0755 -p"
RM="rm -f"
XSLTPROC="xsltproc --nonet --xinclude"


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
	echo "Usage: docbook.sh [-i|-u][-P prefix] target" 1>&2
	return 1
}


#main
install=0
uninstall=0
while getopts "iuP:" "name"; do
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
shift $((OPTIND - 1))
if [ $# -eq 0 ]; then
	_usage
	exit $?
fi

[ -z "$DATADIR" ] && DATADIR="$PREFIX/share"
[ -z "$MANDIR" ] && MANDIR="$DATADIR/man"

while [ $# -gt 0 ]; do
	target="$1"
	source="${target%.*}.xml"
	shift

	#determine the type
	ext="${target##*.}"
	ext="${ext##.}"
	case "$ext" in
		html)
			XSL="http://docbook.sourceforge.net/release/xsl/current/html/docbook.xsl"
			[ -f "${target%.*}.xsl" ] && XSL="${target%.*}.xsl"
			instdir="$DATADIR/doc/$ext/$PACKAGE"
			;;
		1|2|3|4|5|6|7|8|9)
			XSL="http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl"
			instdir="$MANDIR/man$ext"
			;;
		*)
			echo "$0: $target: Unknown type" 1>&2
			exit 2
			;;
	esac

	#uninstall
	if [ "$uninstall" -eq 1 ]; then
		$DEBUG $RM -- "$instdir/$target"		|| exit 2
		continue
	fi

	#install
	if [ "$install" -eq 1 ]; then
		$DEBUG $MKDIR -- "$instdir"			|| exit 2
		$DEBUG $INSTALL -- "$target" "$instdir/$target"	|| exit 2
		continue
	fi

	#create
	$DEBUG $XSLTPROC -o "$target" "$XSL" "$source"
	#XXX ignore errors
	if [ $? -ne 0 ]; then
		echo "$0: $target: Could not create page" 1>&2
		$RM -- "$target"
		break
	fi
done
