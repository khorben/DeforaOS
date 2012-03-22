#!/bin/sh
#$Id$
#Copyright (c) 2010-2012 Pierre Pronchery <khorben@defora.org>
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
CC="../../../../Devel/src/c99/src/c99"
DOT="dot -Tpng:cairo:gd"
RM="rm -f"
VIEW="view"


#functions
#main
$RM ../src/*.o.png
(cd '../src' && make -k CC="$CC" CPPFLAGS="-M graph -D__ELF__ -D__i386__" CFLAGS= distclean all)
for i in ../src/*.o; do
	$DOT -o "$i.png" "$i"
done
$VIEW ../src/*.o.png
