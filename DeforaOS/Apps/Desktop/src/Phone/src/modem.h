/* $Id$ */
/* Copyright (c) 2010-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef PHONE_MODEM_H
# define PHONE_MODEM_H

# include <System.h>
# include "../include/Phone/modem.h"


/* Modem */
/* public */
/* types */
typedef void (*ModemEventCallback)(void * priv, ModemEvent * event);


/* functions */
Modem * modem_new(Config * config, char const * plugin, unsigned int retry);
void modem_delete(Modem * modem);

/* accessors */
ModemConfig * modem_get_config(Modem * modem);
char const * modem_get_name(Modem * modem);
void modem_set_callback(Modem * modem, ModemEventCallback callback,
		void * priv);

/* useful */
int modem_request(Modem * modem, ModemRequest * request);
int modem_request_type(Modem * modem, ModemRequestType type, ...);
int modem_start(Modem * modem);
int modem_stop(Modem * modem);
int modem_trigger(Modem * modem, ModemEventType event);

#endif /* !PHONE_MODEM_H */
