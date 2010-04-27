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
#include <signal.h>
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


/* variables */
static Phone * _phone;


/* functions */
/* usage */
static int _usage(void)
{
	fputs(_("Usage: phone -b baudrate -d device -r retry\n"), stderr);
	return 1;
}


/* main */
static void _main_sigusr1(int signum);
static void _main_sigusr2(int signum);

int main(int argc, char * argv[])
{
	int o;
	Phone * phone;
	char const * device = NULL;
	unsigned int baudrate = 115200;
	int retry = -1;
	char * p;
	struct sigaction sa;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "b:d:r:")) != -1)
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
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	if((phone = phone_new(device, baudrate, retry)) == NULL)
		return 2;
	_phone = phone;
	sa.sa_handler = _main_sigusr1;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGUSR1, &sa, NULL) == -1)
		phone_error(NULL, "sigaction", 0);
	sa.sa_handler = _main_sigusr2;
	if(sigaction(SIGUSR2, &sa, NULL) == -1)
		phone_error(NULL, "sigaction", 0);
	gtk_main();
	_phone = NULL;
	phone_delete(phone);
	return 0;
}

static void _main_sigusr1(int signum)
{
	phone_show_dialer(_phone, TRUE);
}

static void _main_sigusr2(int signum)
{
	phone_show_messages(_phone, TRUE);
}
