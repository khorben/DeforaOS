/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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



#include <System.h>
#include "Panel.h"


/* Template */
/* private */
/* types */
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;
} Template;


/* prototypes */
static Template * _template_init(PanelAppletHelper * helper,
		GtkWidget ** widget);
static void _template_destroy(Template * template);


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Template",
	NULL,
	NULL,
	_template_init,
	_template_destroy,
	NULL,
	FALSE,
	TRUE
};


/* private */
/* functions */
/* template_init */
static Template * _template_init(PanelAppletHelper * helper,
		GtkWidget ** widget)
{
	Template * template;

	if((template = object_new(sizeof(*template))) == NULL)
	{
		helper->error(NULL, error_get(), 1);
		return NULL;
	}
	template->helper = helper;
	*widget = gtk_label_new("Template");
	gtk_widget_show(*widget);
	return template;
}


/* template_destroy */
static void _template_destroy(Template * template)
{
	object_delete(template);
}
