/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#ifndef DESKTOP_PHONE_PLUGIN_H
# define DESKTOP_PHONE_PLUGIN_H

# include "phone.h"


/* PhonePlugin */
/* types */
typedef struct _PhonePlugin PhonePlugin;

typedef void (PhoneConfigForeachCallback)(char const * variable,
		char const * value, void * priv);

typedef struct _PhonePluginHelper
{
	Phone * phone;
	void (*config_foreach)(Phone * phone, char const * section,
			PhoneConfigForeachCallback callback, void * priv);
	char const * (*config_get)(Phone * phone, char const * section,
			char const * variable);
	int (*config_set)(Phone * phone, char const * section,
			char const * variable, char const * value);
	int (*confirm)(Phone * phone, char const * message);
	int (*error)(Phone * phone, char const * message, int ret);
	void (*about_dialog)(Phone * phone);
	int (*event)(Phone * phone, PhoneEvent * event);
	void (*message)(Phone * phone, PhoneMessage message, ...);
	int (*request)(Phone * phone, ModemRequest * request);
	int (*trigger)(Phone * phone, ModemEventType event);
} PhonePluginHelper;

struct _PhonePlugin
{
	PhonePluginHelper * helper;
	char const * name;
	char const * icon;
	int (*init)(PhonePlugin * plugin);
	void (*destroy)(PhonePlugin * plugin);
	int (*event)(PhonePlugin * plugin, PhoneEvent * event);
	void (*settings)(PhonePlugin * plugin);
	void * priv;
};

#endif /* !DESKTOP_PHONE_PLUGIN_H */
