/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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
#include <stdlib.h>


/* Separator */
/* private */
/* types */
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;
} Separator;


/* prototypes */
/* plug-in */
static Separator * _separator_init(PanelAppletHelper * helper,
		GtkWidget ** widget);
static void _separator_destroy(Separator * separator);


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Separator",
	NULL,
	NULL,
	_separator_init,
	_separator_destroy,
	NULL,
	FALSE,
	TRUE
};


/* private */
/* functions */
static Separator * _separator_init(PanelAppletHelper * helper,
		GtkWidget ** widget)
{
	Separator * separator;
	GtkWidget * ret;

	if((separator = malloc(sizeof(*separator))) == NULL)
		return NULL;
	separator->helper = helper;
	ret = gtk_vseparator_new();
	gtk_widget_show(ret);
	*widget = ret;
	return separator;
}


/* separator_destroy */
static void _separator_destroy(Separator * separator)
{
	free(separator);
}
