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



#ifndef DESKTOP_LOCKER_PLUGIN_H
# define DESKTOP_LOCKER_PLUGIN_H

# include "locker.h"


/* LockerPlugin */
/* public */
/* types */
typedef const struct _LockerPluginDefinition LockerPluginDefinition;

typedef struct _LockerPlugin LockerPlugin;

typedef struct _LockerPluginHelper
{
	Locker * locker;
	int (*error)(Locker * locker, char const * message, int ret);
	void (*about_dialog)(Locker * locker);
	int (*action)(Locker * locker, LockerAction action);
	char const * (*config_get)(Locker * locker, char const * section,
			char const * variable);
	int (*config_set)(Locker * locker, char const * section,
			char const * variable, char const * value);
} LockerPluginHelper;

struct _LockerPluginDefinition
{
	char const * name;
	char const * icon;
	char const * description;
	LockerPlugin * (*init)(LockerPluginHelper * helper);
	void (*destroy)(LockerPlugin * plugin);
	int (*event)(LockerPlugin * plugin, LockerEvent event);
};

#endif /* !DESKTOP_LOCKER_PLUGIN_H */
