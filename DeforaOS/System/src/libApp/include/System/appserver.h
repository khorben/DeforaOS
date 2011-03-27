/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libApp */
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



#ifndef LIBAPP_APPSERVER_H
# define LIBAPP_APPSERVER_H

# include "event.h"


/* AppServer */
/* types */
typedef struct _AppServer AppServer;
typedef enum _AppServerOptions
{
	ASO_LOCAL = 1,
	ASO_REMOTE = 2
} AppServerOptions;


/* constants */
# define APPSERVER_MAX_ARGUMENTS	4


/* functions */
AppServer * appserver_new(char const * app, int options);
AppServer * appserver_new_event(char const * app, int options, Event * event);
void appserver_delete(AppServer * appserver);

/* accessors */
void * appserver_get_client_id(AppServer * appserver);

/* useful */
int appserver_loop(AppServer * appserver);

#endif /* !LIBAPP_APPSERVER_H */
