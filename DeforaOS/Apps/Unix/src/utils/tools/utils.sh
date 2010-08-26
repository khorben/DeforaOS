#!/bin/sh
#$Id$
#Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS Unix utils
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



#variables
YEAR=`date '+%Y'`


#functions
#includes
includes() {
	BASENAME="$1"
	PROGRAM="$2"

	cat << EOF

/* $BASENAME */
#define main _${PROGRAM}_main
#define _usage _${PROGRAM}_usage
#define _Prefs _${PROGRAM}_Prefs
#define Prefs ${PROGRAM}_Prefs
#define _prefs_parse _${PROGRAM}_prefs_parse
#define _${PROGRAM} __${PROGRAM}
#include "../src/$BASENAME"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _${PROGRAM}
EOF
}


#main
cat > "utils.c" << EOF
/* \$Id\$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include "utilbox.h"

EOF
for i in ../src/*.c; do
	BASENAME=`basename $i`
	PROGRAM=${BASENAME%%.c}
	#there is an exception
	[ "$PROGRAM" = "common" ] && continue
	includes "$BASENAME" "$PROGRAM"
	CALLS="$CALLS
	{ \"$PROGRAM\",	_${PROGRAM}_main	},"
done >> "utils.c"
cat >> "utils.c" << EOF


Call calls[] =
{
$CALLS
	{ NULL,		NULL		}
};
EOF
