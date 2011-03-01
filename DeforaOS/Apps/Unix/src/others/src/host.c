/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix others */
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



#include <unistd.h>
#include <stdio.h>
#include <netdb.h>


/* host */
static int _host_herror(char const * message, int ret);

static int _host(char * hostname)
{
	struct hostent * he;

	if((he = gethostbyname(hostname)) == NULL)
		return _host_herror(hostname, 1);
	printf("%s has address %u.%u.%u.%u\n", hostname,
			(unsigned char)he->h_addr_list[0][0],
			(unsigned char)he->h_addr_list[0][1],
			(unsigned char)he->h_addr_list[0][2],
			(unsigned char)he->h_addr_list[0][3]);
	return 0;
}


/* herror */
static int _host_herror(char const * message, int ret)
{
	struct
	{
		int errno;
		char const * message;
	} em[] = {
		{ HOST_NOT_FOUND,	"Host not found"	},
		{ NO_DATA,		"No data"		},
		{ NO_RECOVERY,		"No recovery"		},
		{ TRY_AGAIN,		"Try again"		},
		{ 0,			"Unknown error"		}
	};
	int i;

	fprintf(stderr, "%s%s%s", "host: ", message, ": ");
	for(i = 0; em[i].errno != 0 && em[i].errno != h_errno; i++);
	fprintf(stderr, "%s\n", em[i].message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: host address\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind != argc - 1)
		return _usage();
	return _host(argv[optind]) == 0 ? 0 : 2;
}
