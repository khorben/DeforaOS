/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
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



#include <stdlib.h>
#include "System/string.h"


/* test */
static int _test(String * s)
{
	size_t len;
	size_t i;

	if((len = string_length(s)) == 0)
		return 2;
	/* string_clear */
	string_clear(s);
	for(i = 0; i <= len; i++)
		if(s[i] != '\0')
			return 2;
	return 0;
}


/* main */
int main(int argc, char * argv[])
{
	int ret = 0;
	String * s;

	if((s = string_new(argv[0])) == NULL)
		return 2;
	ret |= _test(s);
	string_delete(s);
	return 0;
}
