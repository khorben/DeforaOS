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



#environment
DEVNULL="/dev/null"
EMAIL=
FULLNAME=
HOMEPAGE="http://www.defora.org"
ID="@ID@"
LANG="C"
LICENSE=
METHOD=
PACKAGE=
PROJECTCONF="project.conf"
VERSION=
#executables
CKSUM="cksum"
CUT="cut"
DCH="dch"
DPKG_BUILDPACKAGE="dpkg-buildpackage -rfakeroot"
FIND="find"
GREP="grep"
LINTIAN="lintian"
MAKE="make"
MKDIR="mkdir -p"
PKGLINT="pkglint"
RM="rm -f"
RMD160="rmd160"
SHA1="sha1"
SIZE="_size"
TOUCH="touch"
TR="tr"
WC="wc"
YEAR="$(date +%Y)"
#dependencies
DEPEND_desktop=0
DEPEND_docbookxsl=0
DEPEND_gtkdoc=0
DEPEND_pkgconfig=0
DEPEND_xgettext=0
#method-specific
DEBIAN_PREFIX="deforaos-"
PKGSRC_PREFIX="deforaos-"
PKGSRC_ROOT="/usr/pkgsrc"


#functions
#deforaos_package
deforaos_package()
{
	revision="$1"

	_package_guess_name
	if [ $? -ne 0 ]; then
		_error "Could not determine the package name or version"
		return $?
	fi
	[ -z "$METHOD" ] && _package_guess_method
	[ -z "$LICENSE" ] && _package_guess_license
	_package_guess_dependencies
	[ -z "$EMAIL" ] && _package_guess_email
	[ -z "$FULLNAME" ] && _package_guess_fullname

	#call the proper packaging function
	case "$METHOD" in
		debian|pkgsrc)
			_info "Packaging for $METHOD"
			_package_$METHOD "$revision"
			[ $? -ne 0 ] && return 2
			;;
		*)
			_error "Unsupported packaging method"
			return $?
			;;
	esac

	_info "DeforaOS $PACKAGE $VERSION-$revision packaged"
}

_package_guess_dependencies()
{
	#desktop database
	DEPEND_desktop=0
	for i in data/*.desktop; do
		[ ! -f "$i" ] && continue
		DEPEND_desktop=1
	done

	#docbook-xsl
	DEPEND_docbookxsl=0
	[ -f "doc/docbook.sh" ] && DEPEND_docbookxsl=1

	#gtk-doc
	DEPEND_gtkdoc=0
	[ -f "doc/gtkdoc.sh" ] && DEPEND_gtkdoc=1

	#pkg-config
	DEPEND_pkgconfig=0
	$GREP "\`pkg-config " "src/$PROJECTCONF" > "$DEVNULL" &&
		DEPEND_pkgconfig=1

	#xgettext
	DEPEND_xgettext=0
	[ -f "po/gettext.sh" ] && DEPEND_xgettext=1
}

_package_guess_email()
{
	EMAIL="$USER@defora.org"
}

_package_guess_fullname()
{
	FULLNAME="$USER"
}

_package_guess_license()
{
	[ ! -f "COPYING" ]					&& return 2

	#guess the license
	sum=$($CKSUM COPYING)
	sum=${sum%% *}
	case "$sum" in
		199341746)
			LICENSE="GNU GPL 3"
			;;
	esac
}

_package_guess_method()
{
	#guess the packaging method
	METHOD=

	#debian
	[ -f "/etc/debian_version" ] && METHOD="debian"

	#pkgsrc
	[ -d "/usr/pkg" ] && METHOD="pkgsrc"

	if [ -z "$METHOD" ]; then
		_error "Unsupported platform"
		return $?
	fi
}

_package_guess_name()
{
	PACKAGE=
	VERSION=

	while read line; do
		case "$line" in
			"package="*)
				PACKAGE="${line#package=}"
				;;
			"version="*)
				VERSION="${line#version=}"
				;;
		esac
	done < "$PROJECTCONF"
	[ -z "$PACKAGE" -o -z "$VERSION" ]			&& return 2
	return 0
}


#package_debian
_package_debian()
{
	pkgname=$(echo "$DEBIAN_PREFIX$PACKAGE" | $TR A-Z a-z)

	$RM -r -- "debian"					|| return 2
	$MKDIR -- "debian"					|| return 2

	#check the license
	license=
	case "$LICENSE" in
		"GNU GPL 3")
			license="GPL-3"
			;;
	esac
	[ -z "$license" ] && _warning "Unknown license"

	#debian files
	for i in compat control copyright rules; do
		_info "Creating debian/$i..."
		"_debian_$i" > "debian/$i"
		if [ $? -ne 0 ]; then
			$RM -r -- "debian"
			_error "Could not create debian/$i"
			return 2
		fi
	done

	#debian/changelog
	_info "Creating debian/changelog..."
	_debian_changelog
	if [ $? -ne 0 ]; then
		$RM -r -- "debian"
		_error "Could not create debian/changelog"
		return 2
	fi

	#debian/install
	_debian_install

	#debian/menu
	_debian_menu

	#build the package
	_info "Building the package..."
	$DPKG_BUILDPACKAGE
	#XXX ignore errors if the command is not installed
	if [ $? -eq 127 ]; then
		_warning "Could not build the package"
		return 0
	fi

	#check the package
	_debian_lintian
}

_debian_changelog()
{
	[ -z "$DEBFULLNAME" ] && DEBFULLNAME="$FULLNAME"
	[ -z "$DEBEMAIL" ] && DEBEMAIL="$EMAIL"
	DEBFULLNAME="$DEBFULLNAME" DEBEMAIL="$DEBEMAIL" $DCH --create \
		--distribution "unstable" \
		--package "$pkgname" --newversion "$VERSION-$revision" \
		"Package generated automatically by deforaos-package.sh"
	ret=$?

	#XXX ignore errors if the command is not installed
	if [ $ret -eq 127 ]; then
		_warning "Could not create debian/changelog"
		return 0
	fi

	return $ret
}

_debian_compat()
{
	echo "7"
}

_debian_control()
{
	section="unknown"

	#library major
	major=
	if [ -z "${PACKAGE%%lib*}" ]; then
		major=0
		section="libs"
	fi

	#build dependencies
	depends="debhelper (>= 7.0.50~)"
	[ $DEPEND_docbookxsl -eq 1 ] && depends="$depends, docbook-xsl"
	[ $DEPEND_xgettext -eq 1 ] && depends="$depends, gettext"
	[ $DEPEND_pkgconfig -eq 1 ] && depends="$depends, pkg-config"

	cat << EOF
Source: $pkgname
Section: $section
Priority: extra
Maintainer: $FULLNAME <$EMAIL>
Build-Depends: $depends
Standards-Version: 3.8.4
Homepage: $HOMEPAGE/os/project/$ID/$PACKAGE

Package: $pkgname$major
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends}
Description: DeforaOS $PACKAGE
  DeforaOS $PACKAGE
EOF

	#also generate a development package if necessary
	[ -n "$major" ] || return 0
	cat << EOF

Package: $pkgname-dev
Section: libdevel
Architecture: any
Depends: $pkgname$major (= \${binary:Version})
Description: DeforaOS $PACKAGE (development files)
  DeforaOS $PACKAGE (development files)
EOF
}

_debian_copyright()
{
	cat << EOF
Format-Specification: http://svn.debian.org/wsvn/dep/web/deps/dep5.mdwn?op=file&rev=135
Name: $pkgname
Maintainer: $FULLNAME <$EMAIL>
Source: $HOMEPAGE/os/project/download/$ID

Copyright: $YEAR $FULLNAME <$EMAIL>
License: $license

Files: debian/*
Copyright: $YEAR $FULLNAME <$EMAIL>
License: $license
EOF
	case "$license" in
		GPL-3)
			cat << EOF

License: GPL-3
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3 of the License.
 .
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 .
 You should have received a copy of the GNU General Public License along
 with this program; if not, see <http://www.gnu.org/licenses/>.
 .
 On Debian systems, the full text of the GNU General Public
 License version 3 can be found in the file
 \`/usr/share/common-licenses/GPL-3'.
EOF
			;;
	esac
}

_debian_install()
{
	major=
	[ -z "${PACKAGE%%lib*}" ] && major=0

	[ -n "$major" ] || return 0

	#FIXME some files may be missed (or absent)
	cat > "debian/$pkgname$major.install" << EOF
usr/lib/lib*.so.*
EOF

	cat > "debian/$pkgname-dev.install" << EOF
usr/include/*
usr/lib/lib*.a
usr/lib/lib*.so
usr/lib/pkgconfig/*.pc
usr/share/gtk-doc/html/*
EOF
}

_debian_lintian()
{
	arch="$(dpkg --print-architecture)"
	major=0

	_info "Checking the package..."
	#XXX only check for the packages built this time
	for i in "../${pkgname}_$VERSION-${revision}_$arch.deb" \
		"../$pkgname$major_$VERSION-${revision}_$arch.deb" \
		"../$pkgname-dev_$VERSION-${revision}_$arch.deb"; do
		[ ! -f "$i" ] && continue

		$LINTIAN "$i"
		#XXX ignore errors if the command is not installed
		if [ $? -eq 127 ]; then
			_warning "Could not check the package"
			return 0
		fi
	done
}

_debian_menu()
{
	#obtain the menu entries
	menus=
	for i in data/*.desktop; do
		[ ! -f "$i" ] && continue
		i="${i#data/}"
		i="${i%.desktop}"
		menus="$menus $i"
	done
	[ -z "$menus" ] && return 0

	#debian/menu
	_info "Creating debian/menu..."
	$TOUCH "debian/menu"					|| return 2
	for i in $menus; do
		section="Applications"
		title="$i"
		command="/usr/bin/$i"

		while read line; do
			name="${line%%=*}"
			value="${line#*=}"

			#determine the most essential values
			case "$name" in
				Categories)
					;;
				Exec)
					command="/usr/bin/$value"
					continue
					;;
				Name)
					title="$value"
					continue
					;;
				*)
					continue
					;;
			esac

			#determine the section
			case "$value" in
				*Accessibility*)
					section="Applications/Accessibility"
					;;
				*Player*)
					section="Applications/Video"
					;;
				*Audio*|*Mixer*)
					section="Applications/Sound"
					;;
				*ContactManagement*)
					section="Applications/Data Management"
					;;
				*Development*)
					section="Applications/Programming"
					;;
				*Documentation*)
					section="Help"
					;;
				*Email*|*Telephony*)
					section="Applications/Network/Communication"
					;;
				*FileManager*)
					section="Applications/File Management"
					;;
				*Viewer*)
					section="Applications/Viewers"
					;;
				*Office*)
					section="Applications/Office"
					;;
				*Settings*)
					section="Applications/System/Administration"
					;;
				*TextEditor*)
					section="Applications/Editors"
					;;
				*WebBrowser*)
					section="Applications/Network/Web Browsing"
					;;
			esac
		done < "data/$i.desktop"
		echo "?package($pkgname):needs=\"X11\" \\"
		echo "	section=\"$section\" \\"
		echo "	title=\"$title\" command=\"$command\""
	done >> "debian/menu"
}

_debian_rules()
{
	destdir="\$(PWD)/debian/$pkgname"

	[ -z "${PACKAGE%%lib*}" ] && destdir="\$(PWD)/debian/tmp"
	cat << EOF
#!/usr/bin/make -f
# -*- makefile -*-
# Sample debian/rules that uses debhelper.
# This file was originally written by Joey Hess and Craig Small.
# As a special exception, when this file is copied by dh-make into a
# dh-make output file, you may use that output file without restriction.
# This special exception was added by Craig Small in version 0.37 of dh-make.

# Uncomment this to turn on verbose mode.
#export DH_VERBOSE=1

# These are used for cross-compiling and for saving the configure script
# from having to guess our platform (since we know it already)
DEB_HOST_GNU_TYPE   ?= \$(shell dpkg-architecture -qDEB_HOST_GNU_TYPE)
DEB_BUILD_GNU_TYPE  ?= \$(shell dpkg-architecture -qDEB_BUILD_GNU_TYPE)
ifneq (\$(DEB_HOST_GNU_TYPE),\$(DEB_BUILD_GNU_TYPE))
CC=\$(DEB_HOST_GNU_TYPE)-gcc

override_dh_auto_build:
	\$(MAKE) PREFIX="/usr" CC="\$(CC)"
else
override_dh_auto_build:
	\$(MAKE) PREFIX="/usr"
endif

%:
	dh \$@

override_dh_auto_install:
	\$(MAKE) DESTDIR="$destdir" PREFIX="/usr" install
EOF
}


#package_pkgsrc
_package_pkgsrc()
{
	revision="$1"

	#the archive is needed
	_info "Checking the source archive..."
	if [ ! -f "$PACKAGE-$VERSION.tar.gz" ]; then
		_error "The source archive could not be found"
		_error "Have you ran deforaos-release.sh first?"
		return 2
	fi

	distname="$PACKAGE-$VERSION"
	pkgname=$(echo "$PKGSRC_PREFIX$PACKAGE" | $TR A-Z a-z)

	$RM -r -- "pkgname"					|| return 2
	$MKDIR -- "$pkgname"					|| return 2

	#check the license
	license=
	case "$LICENSE" in
		"GNU GPL 3")
			license="gnu-gpl-v3"
			;;
	esac
	[ -z "$license" ] && _warning "Unknown license"

	#DESCR
	_info "Creating $pkgname/DESCR..."
	_pkgsrc_descr > "$pkgname/DESCR"
	if [ $? -ne 0 ]; then
		$RM -r -- "$pkgname"
		_error "Could not create $pkgname/DESCR"
		return 2
	fi

	#Makefile
	_info "Creating $pkgname/Makefile..."
	_pkgsrc_makefile > "$pkgname/Makefile"
	if [ $? -ne 0 ]; then
		$RM -r -- "$pkgname"
		_error "Could not create $pkgname/Makefile"
		return 2
	fi

	#PLIST
	_info "Creating $pkgname/PLIST..."
	tmpdir="$PWD/$pkgname/destdir"
	$MAKE DESTDIR="$tmpdir" PREFIX="/usr/local" install
	if [ $? -ne 0 ]; then
		$RM -r -- "$pkgname"
		_error "Could not install files in staging directory"
		return 2
	fi
	echo "@comment \$NetBSD\$" > "$pkgname/PLIST"
	(cd "$tmpdir/usr/local" && $FIND . -type f | $CUT -c 3-) >> "$pkgname/PLIST"
	$RM -r -- "$tmpdir"

	#distinfo
	_info "Creating $pkgname/distinfo..."
	_pkgsrc_distinfo > "$pkgname/distinfo"
	if [ $? -ne 0 ]; then
		$RM -r -- "$pkgname"
		_error "Could not create $pkgname/distinfo"
		return 2
	fi

	#check the package
	_info "Running pkglint..."
	#XXX ignore errors for now
	(cd "$pkgname" && $PKGLINT)

	#FIXME:
	#- build the package
	#- review the differences (if any)
	#- commit
}

_pkgsrc_descr()
{
	if [ -f "$PKGSRC_ROOT/wip/$pkgname/DESCR" ]; then
		cat "$PKGSRC_ROOT/wip/$pkgname/DESCR"
		return $?
	fi
	echo "DeforaOS $PACKAGE"
}

_pkgsrc_distinfo()
{
	cat << EOF
\$NetBSD\$

EOF
	$SHA1 -- "$PACKAGE-$VERSION.tar.gz"
	$RMD160 -- "$PACKAGE-$VERSION.tar.gz"
	$SIZE -- "$PACKAGE-$VERSION.tar.gz"
	#additional patches
	for i in "$PKGSRC_ROOT/wip/$pkgname/patches/patch-"*; do
		[ ! -f "$i" ] && continue
		case "$i" in
			*.orig)
				continue
				;;
		esac
		i="${i##*/}"
		(cd "$PKGSRC_ROOT/wip/$pkgname/patches" && $SHA1 -- "$i") ||
			return 2
	done
}

_pkgsrc_makefile()
{
	cat << EOF
# \$NetBSD\$

DISTNAME=	$distname
PKGNAME=	$pkgname-$VERSION
EOF
	[ $revision -gt 0 ] && cat << EOF
PKGREVISION=	$revision
EOF
	cat << EOF
CATEGORIES=	wip
MASTER_SITES=	$HOMEPAGE/os/download/download/$ID/

MAINTAINER=	$EMAIL
HOMEPAGE=	$HOMEPAGE/
COMMENT=	DeforaOS $PACKAGE
EOF

	#license
	[ -n "$license" ] && cat << EOF

LICENSE=	$license
EOF

	#tools
	tools=
	[ $DEPEND_pkgconfig -eq 1 ] && tools="$tools pkg-config"
	[ $DEPEND_xgettext -eq 1 ] && tools="$tools xgettext"
	[ -n "$tools" ] && echo
	for i in $tools; do
		echo "USE_TOOLS+=	$i"
	done

	#build dependencies
	#docbook
	[ $DEPEND_docbookxsl -eq 1 ] && cat << EOF

BUILD_DEPENDS+=	libxslt-[0-9]*:../../textproc/libxslt
BUILD_DEPENDS+=	docbook-xsl-[0-9]*:../../textproc/docbook-xsl
EOF

	cat << EOF

MAKE_FLAGS+=	PREFIX=\${PREFIX}
MAKE_FLAGS+=	DESTDIR=\${DESTDIR}
EOF

	#rc.d scripts
	rcd=
	for i in "$PKGSRC_ROOT/wip/$pkgname/files/"*.sh; do
		[ ! -f "$i" ] && continue
		i="${i##*/}"
		rcd="$rcd ${i%.sh}"
	done
	[ -n "$rcd" ] && echo
	for i in $rcd; do
		echo "RCD_SCRIPTS+=	$i"
	done

	[ -f "$PKGSRC_ROOT/wip/$pkgname/options.mk" ] && cat << EOF

.include "options.mk"
EOF

	echo ""
	[ $DEPEND_gtkdoc -eq 1 ] &&
		echo '.include "../../textproc/gtk-doc/buildlink3.mk"'
	[ $DEPEND_desktop -eq 1 ] &&
		echo '.include "../../sysutils/desktop-file-utils/desktopdb.mk"'
	echo '.include "../../mk/bsd.pkg.mk"'
}


#error
_error()
{
	echo "deforaos-package.sh: error: $@" 1>&2
	return 2
}


#info
_info()
{
	echo "deforaos-package.sh: $@"
}


#size
_size()
{
	while getopts "" name; do
		:
	done
	shift $((OPTIND - 1))
	[ $# -ne 1 ]						&& return 2
	size=$($WC -c "$1")
	for i in $size; do
		size="$i"
		break
	done
	echo "Size ($1) = $size bytes"
}


#usage
_usage()
{
	echo "Usage: deforaos-package.sh [-e e-mail][-i id][-l license][-m method][-n name] revision" 1>&2
	return 1
}


#warning
_warning()
{
	echo "deforaos-package.sh: warning: $@" 1>&2
}


#main
while getopts "e:i:l:m:n:" name; do
	case "$name" in
		e)
			EMAIL="$OPTARG"
			;;
		i)
			ID="$OPTARG"
			;;
		l)
			LICENSE="$OPTARG"
			;;
		m)
			METHOD="$OPTARG"
			;;
		n)
			FULLNAME="$OPTARG"
			;;
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
revision="$1"

deforaos_package "$revision"
