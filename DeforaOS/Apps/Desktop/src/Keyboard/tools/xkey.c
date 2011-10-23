/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
#define XK_LATIN1
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <X11/extensions/XTest.h>


/* private */
/* prototypes */
static int _xkey(char const * display, char const * name);

static int _error(char const * name, char const * message, int ret);
static int _usage(void);


/* functions */
/* xkey */
static int _xkey(char const * display, char const * name)
{
	Display * d;
	KeySym keysym;
	KeyCode keycode;

	if((keysym = XStringToKeysym(name)) == NoSymbol)
		return -_error(name, "Unknown key", 1);
	if((d = XOpenDisplay(NULL)) == NULL)
		return -1;
	keycode = XKeysymToKeycode(d, keysym);
	XTestGrabControl(d, True);
	XTestFakeKeyEvent(d, keycode, True, 0);
	XTestFakeKeyEvent(d, keycode, False, 0);
	XTestGrabControl(d, False);
	XCloseDisplay(d);
	return 0;
}


/* error */
static int _error(char const * name, char const * message, int ret)
{
	fprintf(stderr, "%s: %s%s%s\n", "xkey", (name != NULL) ? name : "",
			(name != NULL) ? ": " : "", message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: xkey [-d display] name\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * display = NULL;

	while((o = getopt(argc, argv, "d:")) != -1)
		switch(o)
		{
			case 'd':
				display = optarg;
				break;
			default:
				return _usage();
		}
	if(optind + 1 != argc)
		return _usage();
	return (_xkey(display, argv[optind]) == 0) ? 0 : 2;
}
