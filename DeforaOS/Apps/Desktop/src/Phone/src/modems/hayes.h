/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#ifndef DESKTOP_PHONE_MODEM_HAYES_H
# define DESKTOP_PHONE_MODEM_HAYES_H


/* Hayes */
/* types */
typedef enum _HayesRequestType
{
	HAYES_REQUEST_COMMAND_QUEUE	= 0
} HayesRequestType;

typedef union _HayesRequest
{
	/* COMMAND_QUEUE */
	struct
	{
		char const * command;
	} command_queue;
} HayesRequest;

#endif /* !DESKTOP_PHONE_MODEM_HAYES_H */
