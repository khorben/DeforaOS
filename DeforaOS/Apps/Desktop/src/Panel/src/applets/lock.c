/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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



#include "Panel.h"


/* Lock */
/* private */
/* prototypes */
static GtkWidget * _lock_init(PanelApplet * applet);

/* callbacks */
static void _on_clicked(void);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_lock_init,
	NULL,
	PANEL_APPLET_POSITION_START,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* lock_init */
static GtkWidget * _lock_init(PanelApplet * applet)
{
	GtkWidget * ret;
	GtkWidget * image;

	ret = gtk_button_new();
	image = gtk_image_new_from_icon_name("gnome-lockscreen",
			applet->helper->icon_size);
	gtk_button_set_image(GTK_BUTTON(ret), image);
	gtk_button_set_relief(GTK_BUTTON(ret), GTK_RELIEF_NONE);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(ret, "Lock screen");
#endif
	g_signal_connect(G_OBJECT(ret), "clicked", G_CALLBACK(_on_clicked),
			NULL);
	return ret;
}


/* callbacks */
/* on_clicked */
static void _on_clicked(void)
{
	char * argv[] = { "xscreensaver-command", "-lock", NULL };
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH
		| G_SPAWN_STDOUT_TO_DEV_NULL
		| G_SPAWN_STDERR_TO_DEV_NULL;

	g_spawn_async(NULL, argv, NULL, flags, NULL, NULL, NULL, NULL);
}
