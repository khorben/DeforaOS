/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
typedef struct _LockerPlugin LockerPlugin;

typedef struct _LockerPluginHelper
{
	Locker * locker;
	int (*error)(Locker * locker, char const * message, int ret);
	void (*about_dialog)(Locker * locker);
	void (*action)(Locker * locker, LockerAction action);
} LockerPluginHelper;

struct _LockerPlugin
{
	LockerPluginHelper * helper;
	char const * name;
	char const * icon;
	int (*init)(LockerPlugin * plugin);
	void (*destroy)(LockerPlugin * plugin);
	void (*event)(LockerPlugin * plugin, LockerEvent event);
	void * priv;
};

#endif /* !DESKTOP_LOCKER_PLUGIN_H */
