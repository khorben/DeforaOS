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



#ifndef LIBAPP_APPCLIENT_H
# define LIBAPP_APPCLIENT_H

# include <stdint.h>
# include <System/event.h>


/* AppClient */
/* types */
typedef struct _AppClient AppClient;


/* functions */
AppClient * appclient_new(char const * service);
AppClient * appclient_new_event(char const * service, Event * event);
void appclient_delete(AppClient * appclient);

/* useful */
int appclient_call(AppClient * appclient, int32_t * ret, char const * function,
		...);

#endif /* !LIBAPP_APPCLIENT_H */
