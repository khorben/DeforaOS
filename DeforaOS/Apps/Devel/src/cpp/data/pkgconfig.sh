#!/bin/sh
#$Id$
#Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
#All rights reserved.
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
. "../config.sh"
DEBUG="_debug"
DEVNULL="/dev/null"
INSTALL="install -m 0644"
MKDIR="mkdir -p"
PREFIX="/usr/local"
RM="rm -f"
SED="sed"


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
	echo "Usage: pkgconfig.sh [-i|-u][-P prefix] target" 1>&2
	return 1
}


#main
install=0
uninstall=0
while getopts iuP: name; do
	case $name in
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

PKGCONFIG="$PREFIX/lib/pkgconfig"
while [ $# -gt 0 ]; do
	target="$1"
	shift

	#uninstall
	if [ "$uninstall" -eq 1 ]; then
		$DEBUG $RM -- "$PKGCONFIG/$target"		|| exit 2
		continue
	fi

	#install
	if [ "$install" -eq 1 ]; then
		$DEBUG $MKDIR -- "$PKGCONFIG"			|| exit 2
		$DEBUG $INSTALL "$target" "$PKGCONFIG/$target"	|| exit 2
		continue
	fi

	#create
	$SED -e "s,PREFIX,$PREFIX," -e "s,VERSION,$VERSION," "$target.in" \
		> "$target"
	if [ $? -ne 0 ]; then
		$RM -- "$target" 2> "$DEVNULL"
		exit 2
	fi
done
