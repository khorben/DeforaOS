/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef LIBAPP_APP_APPMESSAGE_H
# define LIBAPP_APP_APPMESSAGE_H

# include <System/buffer.h>
# include <System/string.h>
# include "app.h"


/* AppMessage */
/* type */
typedef enum _AppMessageType
{
	AMT_CALL = 0
} AppMessageType;


/* functions */
AppMessage * appmessage_new_call(char const * method, ...);
AppMessage * appmessage_new_deserialize(Buffer * buffer);
void appmessage_delete(AppMessage * appmessage);

/* accessors */
String const * appmessage_get_method(AppMessage * message);
AppMessageType appmessage_get_type(AppMessage * message);

/* useful */
int appmessage_serialize(AppMessage * message, Buffer * buffer);

#endif /* !LIBAPP_APP_APPMESSAGE_H */
