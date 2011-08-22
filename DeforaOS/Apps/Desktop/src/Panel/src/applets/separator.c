/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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


/* separator */
/* private */
/* prototypes */
/* plug-in */
static GtkWidget * _separator_init(PanelApplet * applet);
static void _separator_destroy(PanelApplet * applet);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	"Separator",
	NULL,
	_separator_init,
	_separator_destroy,
	NULL,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
static GtkWidget * _separator_init(PanelApplet * applet)
{
	GtkWidget * widget;

	widget = gtk_vseparator_new();
	applet->priv = widget;
	gtk_widget_show(widget);
	return widget;
}


/* separator_destroy */
static void _separator_destroy(PanelApplet * applet)
{
	GtkWidget * widget = applet->priv;

	gtk_widget_destroy(widget);
}
