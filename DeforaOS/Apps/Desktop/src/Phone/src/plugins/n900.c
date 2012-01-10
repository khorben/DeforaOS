/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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
/* This code is inspired by existing work by Sebastian Reichel <sre@debian.org>,
 * see https://elektranox.org/n900/libisi */
/* The N900 icon is adapted from an original work by Luc Andrea, CC BY-SA 3.0,
 * see http://en.wikipedia.org/wiki/File:Nokia_N900_Maemo_icon.svg */
/* TODO:
 * - test on actual hardware */



#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <System.h>
#include "Phone.h"


/* N900 */
/* types */
typedef struct _PhonePlugin
{
	PhonePluginHelper * helper;
} N900;


/* prototypes */
/* plug-in */
static N900 * _n900_init(PhonePluginHelper * helper);
static void _n900_destroy(N900 * n900);
static int _n900_event(PhonePlugin * plugin, PhoneEvent * event);


/* public */
/* variables */
PhonePluginDefinition plugin =
{
	"Nokia N900",
	"phone-n900",
	NULL,
	_n900_init,
	_n900_destroy,
	_n900_event,
	NULL
};


/* private */
/* functions */
/* plug-in */
/* n900_init */
static N900 * _n900_init(PhonePluginHelper * helper)
{
	N900 * n900;

	if((n900 = object_new(sizeof(*n900))) == NULL)
		return NULL;
	n900->helper = helper;
	return n900;
}


/* n900_destroy */
static void _n900_destroy(N900 * n900)
{
	object_delete(n900);
}


/* n900_event */
static int _event_power_on(PhonePlugin * plugin, gboolean power);

static int _n900_event(PhonePlugin * plugin, PhoneEvent * event)
{
	switch(event->type)
	{
		case PHONE_EVENT_TYPE_OFFLINE:
			_event_power_on(plugin, FALSE);
			break;
		case PHONE_EVENT_TYPE_ONLINE:
			_event_power_on(plugin, TRUE);
			break;
		default:
			break;
	}
	return 0;
}

static int _event_power_on(PhonePlugin * plugin, gboolean power)
{
	int ret = 0;
	char const root[] = "/sys/devices/platform/gpio-switch";
	struct
	{
		char const * name;
		int enable;
		int disable;
		unsigned int delay;
	} commands[] = {
		{ "cmt_apeslpx",	0,	0,	0	},
		{ "cmt_rst_rq",		0,	0,	0	},
		{ "cmt_bsi",		0,	-1,	0	},
		{ "cmt_rst",		0,	0,	0	},
		{ "cmt_en",		1,	0,	0	},
		{ "cmt_rst",		1,	1,	0	},
		{ "cmt_rst_eq",		1,	-1,	0	},
		{ "cmt_en",		1,	-1,	5	},
	};
	size_t i;
	int fd;
	char path[256];
	char buf[256];
	int len;

	for(i = 0; i < sizeof(commands) / sizeof(*commands) && ret == 0; i++)
	{
		if((power ? commands[i].enable : commands[i].disable) < 0)
			continue;
		if(commands[i].delay > 0)
			/* FIXME freezes the application for as long */
			sleep(commands[i].delay);
		snprintf(path, sizeof(path), "%s/%s/%s", root, commands[i].name,
				"state");
		if((fd = open(path, O_WRONLY)) < 0)
		{
			snprintf(buf, sizeof(buf), "%s: %s", path, strerror(
						errno));
			ret = plugin->helper->error(NULL, buf, 1);
			break;
		}
		len = snprintf(buf, sizeof(buf), "%s", power ? "active"
				: "inactive");
		if(write(fd, buf, len) != len)
		{
			snprintf(buf, sizeof(buf), "%s: %s", path,
					strerror(errno));
			ret = plugin->helper->error(NULL, buf, 1);
		}
		close(fd);
	}
	return ret;
}
