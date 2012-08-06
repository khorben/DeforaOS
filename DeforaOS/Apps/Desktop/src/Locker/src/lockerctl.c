/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Locker */
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
#include "../include/Locker.h"
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


/* usage */
static int _usage(void)
{
#ifdef EMBEDDED
	fputs(_("Usage: lockerctl [-D|-E|-S|-l|-s|-u|-z]\n"
"  -D	Temporarily disable the screensaver\n"
"  -E	Enable the screensaver again\n"
"  -S	Display or change settings\n"
"  -l	Lock the screen\n"
"  -s	Activate the screen saver\n"
"  -u	Unlock the screen\n"
"  -z	Suspend the device\n"), stderr);
#else
	fputs(_("Usage: lockerctl [-D|-E|-S|-l|-s|-u|-z]\n"
"  -D	Temporarily disable the screensaver\n"
"  -E	Enable the screensaver again\n"
"  -S	Display or change settings\n"
"  -l	Lock the screen\n"
"  -s	Activate the screen saver\n"
"  -u	Unlock the screen\n"
"  -z	Suspend the computer\n"), stderr);
#endif
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int action = -1;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "DESlsuz")) != -1)
		switch(o)
		{
			case 'D':
				if(action != -1)
					return _usage();
				action = LOCKER_ACTION_DISABLE;
				break;
			case 'E':
				if(action != -1)
					return _usage();
				action = LOCKER_ACTION_ENABLE;
				break;
			case 'S':
				if(action != -1)
					return _usage();
				action = LOCKER_ACTION_SHOW_PREFERENCES;
				break;
			case 'l':
				if(action != -1)
					return _usage();
				action = LOCKER_ACTION_LOCK;
				break;
			case 's':
				if(action != -1)
					return _usage();
				action = LOCKER_ACTION_ACTIVATE;
				break;
			case 'u':
				if(action != -1)
					return _usage();
				action = LOCKER_ACTION_UNLOCK;
				break;
			case 'z':
				if(action != -1)
					return _usage();
				action = LOCKER_ACTION_SUSPEND;
				break;
			default:
				return _usage();
		}
	if(action == -1 || optind != argc)
		return _usage();
	desktop_message_send(LOCKER_CLIENT_MESSAGE, LOCKER_MESSAGE_ACTION,
			action, TRUE);
	return 0;
}
