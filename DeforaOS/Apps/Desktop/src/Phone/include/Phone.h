/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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



#ifndef PHONE_PHONE_H
# define PHONE_PHONE_H


/* types */
typedef enum _PhoneEvent
{
	PHONE_EVENT_CALL_ESTABLISHED = 0,
	PHONE_EVENT_CALL_INCOMING,
	PHONE_EVENT_CALL_OUTGOING,
	PHONE_EVENT_CALL_TERMINATED,
	PHONE_EVENT_SMS_SENT,		/* char * buffer, size_t * len */
	PHONE_EVENT_SMS_RECEIVED	/* char * buffer, size_t * len */
} PhoneEvent;

typedef struct _PhonePlugin
{
	int (*init)(void);
	int (*destroy)(void);
	void (*event)(PhoneEvent event, ...);
} PhonePlugin;

#endif /* !PHONE_PHONE_H */
