/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Init */
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



#ifndef INIT_SESSION_H
# define INIT_SESSION_H

# include <sys/types.h>
# include <System.h>


/* types */
typedef struct _Session Session;


/* functions */
Session * session_new(char const * name, char const * profile, Event * event);
void session_delete(Session * session);

/* AppInterface */
int session_register(String const * interface, uint16_t port);

/* useful */
int session_reload(Session * session);
int session_start(Session * session);
int session_stop(Session * session);

#endif /* INIT_SESSION_H */
