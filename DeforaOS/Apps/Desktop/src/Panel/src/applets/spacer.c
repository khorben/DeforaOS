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


/* Spacer */
/* private */
/* types */
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;
} Spacer;


/* prototypes */
/* plug-in */
static Spacer * _spacer_init(PanelAppletHelper * helper, GtkWidget ** widget);
static void _spacer_destroy(Spacer * spacer);


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Spacer",
	NULL,
	NULL,
	_spacer_init,
	_spacer_destroy,
	NULL,
	TRUE,
	TRUE
};


/* private */
/* functions */
static Spacer * _spacer_init(PanelAppletHelper * helper, GtkWidget ** widget)
{
	Spacer * spacer;
	GtkWidget * ret;

	if((spacer = malloc(sizeof(*spacer))) == NULL)
		return NULL;
	spacer->helper = helper;
	ret = gtk_label_new(NULL);
	gtk_widget_show(ret);
	*widget = ret;
	return spacer;
}


/* spacer_destroy */
static void _spacer_destroy(Spacer * spacer)
{
	free(spacer);
}
