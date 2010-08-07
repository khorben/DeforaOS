/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "phone.h"
#include "../config.h"
#define _(string) gettext(string)


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR	DATADIR "/locale"
#endif


/* functions */
/* usage */
static int _usage(void)
{
	fputs(_("Usage: phone [-b baudrate][-d device][-r retry][-F]\n"
"  -b	Speed to set before communicating with the device\n"
"  -d	Path to the modem device\n"
"  -r	Delay between two tries to open and settle with the modem (ms)\n"
"  -F	Enable hardware flow control (RTS/CTS)\n"), stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Phone * phone;
	char const * device = NULL;
	unsigned int baudrate = 0;
	int retry = -1;
	int hwflow = -1;
	char * p;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "b:d:r:F")) != -1)
		switch(o)
		{
			case 'b':
				baudrate = strtoul(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			case 'd':
				device = optarg;
				break;
			case 'r':
				retry = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			case 'F':
				hwflow = 1;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	if((phone = phone_new(device, baudrate, retry, hwflow)) == NULL)
		return 2;
	gtk_main();
	phone_delete(phone);
	return 0;
}
