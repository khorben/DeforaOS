#!/bin/sh
#$Id$
#Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS Devel scripts
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, version 3 of the License.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.
#TODO:
#- no longer require config.sh



#environment
DEVNULL="/dev/null"
#executables
CVS="cvs"
MAKE="make"
RM="echo rm -f"
TAR="tar"


#functions
#deforaos_release
deforaos_release()
{
	version="$1"
	PACKAGE=
	VERSION=

	[ -f "./config.sh" ] && . "./config.sh"
	if [ -z "$PACKAGE" -o -z "$VERSION" ]; then
		_error "Could not determine the package name or version"
		return $?
	fi
	_info "Releasing $PACKAGE $VERSION"

	if [ "$version" != "$VERSION" ]; then
		_error "The version does not match"
		return $?
	fi

	_info "Obtaining latest version..."
	$CVS up -A > "$DEVNULL"
	if [ $? -ne 0 ]; then
		_error "Could not update the sources"
		return $?
	fi

	_info "Checking for differences..."
	#XXX this method may be obsoleted in a future version of CVS
	$CVS diff > "$DEVNULL"
	if [ $? -ne 0 ]; then
		_error "The sources were modified"
		return $?
	fi

	_info "Creating the archive..."
	$MAKE dist > $DEVNULL
	if [ $? -ne 0 ]; then
		_error "Could not create the archive"
		return $?
	fi

	#check the archive
	_info "Checking the archive..."
	archive="$PACKAGE-$VERSION.tar.gz"
	$TAR -xzf "$archive" > $DEVNULL &&
		(cd "$PACKAGE-$VERSION" && $MAKE)
	res=$?
	$RM -r "$PACKAGE-$VERSION"
	if [ $res -ne 0 ]; then
		_error "Could not validate the archive"
		return $?
	fi
}


#error
_error()
{
	echo "deforaos-release.sh: $@" 1>&2
	return 2
}


#info
_info()
{
	echo "deforaos-release.sh: $@"
}


#usage
_usage()
{
	echo "Usage: deforaos-release.sh version" 1>&2
	return 1
}


#main
while getopts "" name; do
	case "$name" in
		?)
			_usage
			exit $?
			;;
	esac
done
shift $((OPTIND - 1))
if [ $# -ne 1 ]; then
	_usage
	exit $?
fi
version="$1"

deforaos_release "$version"
