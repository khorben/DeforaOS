/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef LOCKER_LOCKER_H
# define LOCKER_LOCKER_H

# include "../include/Locker.h"


/* Locker */
/* public */
/* constants */
# define LOCKER_CONFIG_FILE	".locker"


/* functions */
Locker * locker_new(char const * demo, char const * auth);
void locker_delete(Locker * locker);

/* useful */
void locker_show_preferences(Locker * locker, gboolean show);

#endif /* !LOCKER_LOCKER_H */
