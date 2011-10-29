#!/usr/bin/env sh
#$Id$
#Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS System libSystem
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


FAILED=
./string		|| FAILED="$FAILED string"
[ -z "$FAILED" ]	&& exit 0
echo "Failed tests:$FAILED" 1>&2
return 2
