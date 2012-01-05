/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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
#include <libintl.h>
#include "Browser.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Template */
/* private */
/* types */
typedef struct _BrowserPlugin
{
	BrowserPluginHelper * helper;

	GtkWidget * widget;

	/* FIXME implement */
} Template;


/* prototypes */
static Template * _template_init(BrowserPluginHelper * helper);
static void _template_destroy(Template * template);
static GtkWidget * _template_get_widget(Template * template);
static void _template_refresh(Template * template, char const * path);


/* public */
/* variables */
BrowserPluginDefinition plugin =
{
	N_("Template"),
	NULL,
	NULL,
	_template_init,
	_template_destroy,
	_template_get_widget,
	_template_refresh
};


/* private */
/* functions */
/* template_init */
static Template * _template_init(BrowserPluginHelper * helper)
{
	Template * template;

	if((template = object_new(sizeof(*template))) == NULL)
		return NULL;
	template->helper = helper;
	/* FIXME implement */
	template->widget = gtk_label_new("Template");
	return template;
}


/* template_destroy */
static void _template_destroy(Template * template)
{
	/* FIXME implement */
	object_delete(template);
}


/* template_get_widget */
static GtkWidget * _template_get_widget(Template * template)
{
	return template->widget;
}


/* template_refresh */
static void _template_refresh(Template * template, char const * path)
{
	/* FIXME implement */
}
