/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop libDesktop */
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



#ifndef LIBDESKTOP_DESKTOP_MESSAGE_H
# define LIBDESKTOP_DESKTOP_MESSAGE_H

# include <stdint.h>


/* Message */
/* types */
typedef int (*DesktopMessageCallback)(void * data, uint32_t value1,
		uint32_t value2, uint32_t value3);


/* functions */
int desktop_message_register(char const * destination,
		DesktopMessageCallback callback, void * data);
int desktop_message_send(char const * destination, uint32_t value1,
		uint32_t value2, uint32_t value3);

#endif /* !LIBDESKTOP_DESKTOP_MESSAGE_H */
