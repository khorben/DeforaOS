/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Network Directory */
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



#ifndef DIRECTORY_DIRECTORY_H
# define DIRECTORY_DIRECTORY_H

# include <System.h>
# include <System/App.h>


/* Directory */
/* types */
typedef struct _Directory Directory;


/* functions */
Directory * directory_new(AppServerOptions options, Event * event);
void directory_delete(Directory * directory);

#endif /* !DIRECTORY_DIRECTORY_H */
