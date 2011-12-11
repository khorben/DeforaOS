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



#ifndef DESKTOP_LOCKER_DEMO_H
# define DESKTOP_LOCKER_DEMO_H

# include "locker.h"


/* LockerDemo */
/* public */
/* types */
typedef struct _LockerDemoHelper
{
	Locker * locker;
	int (*error)(Locker * locker, char const * message, int ret);
	char const * (*config_get)(Locker * locker, char const * section,
			char const * variable);
	int (*config_set)(Locker * locker, char const * section,
			char const * variable, char const * value);
} LockerDemoHelper;

typedef struct _LockerDemo LockerDemo;

struct _LockerDemo
{
	LockerDemoHelper * helper;
	char const * name;
	int (*init)(LockerDemo * demo);
	void (*destroy)(LockerDemo * demo);
	int (*add)(LockerDemo * demo, unsigned int id);
	void (*remove)(LockerDemo * demo, unsigned int id);
	void * priv;
};

#endif /* !DESKTOP_LOCKER_DEMO_H */
