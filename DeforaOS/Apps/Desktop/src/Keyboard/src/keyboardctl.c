/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Keyboard */
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
#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <Desktop.h>
#include "../include/Keyboard.h"
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


/* keyboardctl */
/* private */
/* prototypes */
static int _keyboardctl(KeyboardMessage message, unsigned int arg1);
static int _usage(void);


/* functions */
static int _keyboardctl(KeyboardMessage message, unsigned int arg1)
{
	desktop_message_send(KEYBOARD_CLIENT_MESSAGE, message, arg1, 0);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs(_("Usage: keyboardctl [-H|-S]\n"
"  -H	Hide the keyboard\n"
"  -S	Show the keyboard\n"), stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	int message = -1;
	int arg1;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "HS")) != -1)
		switch(o)
		{
			case 'H':
				message = KEYBOARD_MESSAGE_SET_VISIBLE;
				arg1 = 0;
				break;
			case 'S':
				message = KEYBOARD_MESSAGE_SET_VISIBLE;
				arg1 = 1;
				break;
			default:
				return _usage();
		}
	if(argc != optind || message < 0)
		return _usage();
	return (_keyboardctl(message, arg1) == 0) ? 0 : 2;
}
