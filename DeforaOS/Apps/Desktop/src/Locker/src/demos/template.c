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
#include "Locker/demo.h"


/* Template */
/* private */
/* types */
typedef struct _LockerDemo
{
	LockerDemoHelper * helper;
} Template;


/* prototypes */
/* plug-in */
static Template * _template_init(LockerDemoHelper * helper);
static void _template_destroy(Template * template);
static int _template_add(Template * template, GtkWidget * window);
static void _template_remove(Template * template, GtkWidget * window);
static void _template_start(Template * template);
static void _template_stop(Template * template);


/* public */
/* variables */
/* plug-in */
LockerDemoDefinition plugin =
{
	"Template",
	NULL,
	NULL,
	_template_init,
	_template_destroy,
	_template_add,
	_template_remove,
	_template_start,
	_template_stop
};


/* private */
/* functions */
/* plug-in */
/* template_init */
static Template * _template_init(LockerDemoHelper * helper)
{
	Template * template;

	if((template = object_new(sizeof(*template))) == NULL)
		return NULL;
	template->helper = helper;
	return template;
}


/* template_destroy */
static void _template_destroy(Template * template)
{
	object_delete(template);
}


/* template_add */
static int _template_add(Template * template, GtkWidget * window)
{
	return 0;
}


/* template_remove */
static void _template_remove(Template * template, GtkWidget * window)
{
}


/* template_start */
static void _template_start(Template * template)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
}


/* template_stop */
static void _template_stop(Template * template)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
}
