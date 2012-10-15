/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
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



#include <stdio.h>
#include <string.h>
#include "Mailer.h"


/* email */
static int _email(char const * progname, char const * name, char const * email,
		char const * str)
{
	int ret = 0;
	char * n;
	char * e;

	printf("%s: Testing \"%s\"\n", progname, str);
	n = mailer_helper_get_name(str);
	e = mailer_helper_get_email(str);
	if(strcmp(name, n) != 0)
	{
		fprintf(stderr, "%s: %s: %s\n", progname, n,
				"Does not match the name");
		ret |= 2;
	}
	if(strcmp(email, e) != 0)
	{
		fprintf(stderr, "%s: %s: %s\n", progname, e,
				"Does not match the e-mail");
		ret |= 3;
	}
	free(e);
	free(n);
	return ret;
}


/* main */
int main(int argc, char * argv[])
{
	int ret = 0;

	ret |= _email(argv[0], "John Doe", "john@doe.com",
			"john@doe.com (John Doe)");
	ret |= _email(argv[0], "John Doe", "john@doe.com",
			"John Doe <john@doe.com>");
	return (ret == 0) ? 0 : 2;
}
