/* $Id$ */
/* Copyright (c) 2010-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Graphics GServer */
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



#ifndef GSERVER_GSERVER_H
# define GSERVER_GSERVER_H

# include <System.h>
# include <System/App.h>
# include "GServer/gserver.h"


/* GServer */
/* public */
/* functions */
GServer * gserver_new(AppServerOptions options, Event * event);
void gserver_delete(GServer * gserver);


/* accessors */
Event * gserver_get_event(GServer * gserver);


/* useful */
int gserver_loop(GServer * gserver);
void gserver_refresh(GServer * gserver);

#endif /* !GSERVER_GSERVER_H */
