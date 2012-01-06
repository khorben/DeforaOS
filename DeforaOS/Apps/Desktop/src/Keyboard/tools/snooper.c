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
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>


/* prototypes */
/* callbacks */
static gint _snooper_on_key_snoop(GtkWidget * grab, GdkEventKey * event,
		gpointer data);


/* functions */
/* callbacks */
/* snooper_on_key_snoop */
static gint _snooper_on_key_snoop(GtkWidget * grab, GdkEventKey * event,
		gpointer data)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return TRUE;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: snooper\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	guint id;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	id = gtk_key_snooper_install(_snooper_on_key_snoop, NULL);
	gtk_main();
	gtk_key_snooper_remove(id);
	return 0;
}
