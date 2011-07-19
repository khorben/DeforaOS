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
#define _properties_refresh _properties_do_refresh
#include "../properties.c"
#undef _properties_refresh


/* Properties */
/* private */
/* prototypes */
static GtkWidget * _properties_init(BrowserPlugin * plugin);
static void _properties_destroy(BrowserPlugin * plugin);
static void _properties_refresh(BrowserPlugin * plugin, char const * path);


/* public */
/* variables */
BrowserPlugin plugin =
{
	NULL,
	N_("Properties"),
	GTK_STOCK_PROPERTIES,
	_properties_init,
	_properties_destroy,
	_properties_refresh,
	NULL
};


/* private */
/* functions */
/* properties_init */
static GtkWidget * _properties_init(BrowserPlugin * plugin)
{
	Properties * properties;

	if((properties = _properties_new(NULL, NULL)) == NULL)
		return NULL;
	plugin->priv = properties;
	return _properties_get_view(properties);
}


/* properties_destroy */
static void _properties_destroy(BrowserPlugin * plugin)
{
	Properties * properties = plugin->priv;

	_properties_delete(properties);
}


/* properties_refresh */
static void _properties_refresh(BrowserPlugin * plugin, char const * path)
{
	Properties * properties = plugin->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, path);
#endif
	_properties_set_filename(properties, path);
}
