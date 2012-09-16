/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Locker */
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
#include "Locker.h"


/* Template */
/* private */
/* types */
typedef struct _LockerAuth
{
	LockerAuthHelper * helper;

	/* widgets */
	GtkWidget * widget;
} Template;


/* prototypes */
/* plug-in */
static Template * _template_init(LockerAuthHelper * helper);
static void _template_destroy(Template * template);
static GtkWidget * _template_get_widget(Template * template);
static int _template_action(Template * template, LockerAction action);


/* public */
/* variables */
/* plug-in */
LockerAuthDefinition plugin =
{
	"Template",
	NULL,
	NULL,
	_template_init,
	_template_destroy,
	_template_get_widget,
	_template_action
};


/* private */
/* functions */
/* template_init */
static Template * _template_init(LockerAuthHelper * helper)
{
	Template * template;

	if((template = object_new(sizeof(*template))) == NULL)
		return NULL;
	template->helper = helper;
	template->widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_widget_show(template->widget);
	return template;
}


/* template_destroy */
static void _template_destroy(Template * template)
{
	gtk_widget_destroy(template->widget);
	object_delete(template);
}


/* template_get_widget */
static GtkWidget * _template_get_widget(Template * template)
{
	return template->widget;
}


/* template_action */
static int _template_action(Template * template, LockerAction action)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, action);
#endif
	switch(action)
	{
		case LOCKER_ACTION_DEACTIVATE:
			gtk_widget_show(template->widget);
			break;
		case LOCKER_ACTION_LOCK:
			gtk_widget_hide(template->widget);
			break;
		case LOCKER_ACTION_START:
		case LOCKER_ACTION_UNLOCK:
			gtk_widget_hide(template->widget);
			break;
		default:
			break;
	}
	return 0;
}
