/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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
#include <Desktop.h>
#include "Phone.h"


/* Template */
/* private */
/* types */
typedef struct _PhonePlugin
{
	PhonePluginHelper * helper;
} TemplatePhonePlugin;


/* prototypes */
/* plug-in */
static TemplatePhonePlugin * _template_init(PhonePluginHelper * helper);
static void _template_destroy(TemplatePhonePlugin * template);
static int _template_event(TemplatePhonePlugin * template, PhoneEvent * event);


/* public */
/* variables */
PhonePluginDefinition plugin =
{
	"Template",
	NULL,
	NULL,
	_template_init,
	_template_destroy,
	_template_event,
	NULL
};


/* private */
/* functions */
/* template_init */
static TemplatePhonePlugin * _template_init(PhonePluginHelper * helper)
{
	TemplatePhonePlugin * template;

	if((template = object_new(sizeof(*template))) == NULL)
		return NULL;
	template->helper = helper;
	return template;
}


/* template_destroy */
static void _template_destroy(TemplatePhonePlugin * template)
{
	object_delete(template);
}


/* template_event */
static int _template_event(TemplatePhonePlugin * template, PhoneEvent * event)
{
	switch(event->type)
	{
		default:
			break;
	}
	return 0;
}
