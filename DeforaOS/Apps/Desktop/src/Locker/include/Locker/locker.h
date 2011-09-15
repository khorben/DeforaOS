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



#ifndef DESKTOP_LOCKER_LOCKER_H
# define DESKTOP_LOCKER_LOCKER_H


/* Locker */
/* public */
/* types */
typedef struct _Locker Locker;

typedef enum _LockerAction
{
	LOCKER_ACTION_ACTIVATE = 0,
	LOCKER_ACTION_LOCK,
	LOCKER_ACTION_SHOW_PREFERENCES,
	LOCKER_ACTION_UNLOCK
} LockerAction;


/* constants */
# define LOCKER_CLIENT_MESSAGE	"DEFORAOS_DESKTOP_LOCKER_CLIENT"
# define LOCKER_MESSAGE_ACTION	0

#endif /* !DESKTOP_LOCKER_LOCKER_H */
