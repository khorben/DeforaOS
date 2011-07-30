/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
typedef struct _Template
{
	/* FIXME implement */
} Template;


/* prototypes */
static GtkWidget * _template_init(BrowserPlugin * plugin);
static void _template_destroy(BrowserPlugin * plugin);
static void _template_refresh(BrowserPlugin * plugin, char const * path);


/* public */
/* variables */
BrowserPlugin plugin =
{
	NULL,
	N_("Template"),
	NULL,
	_template_init,
	_template_destroy,
	_template_refresh,
	NULL
};


/* private */
/* functions */
/* template_init */
static GtkWidget * _template_init(BrowserPlugin * plugin)
{
	Template * template;

	if((template = object_new(sizeof(*template))) == NULL)
		return NULL;
	plugin->priv = template;
	/* FIXME implement */
	return gtk_label_new("Template");
}


/* template_destroy */
static void _template_destroy(BrowserPlugin * plugin)
{
	Template * template = plugin->priv;

	object_delete(template);
}


/* template_refresh */
static void _template_refresh(BrowserPlugin * plugin, char const * path)
{
	Template * template = plugin->priv;

	/* FIXME implement */
}
