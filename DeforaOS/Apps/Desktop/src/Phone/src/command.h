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



#ifndef PHONE_COMMAND_H
# define PHONE_COMMAND_H

# include "gsm.h"


/* GSMCommand */
/* public */
/* types */
typedef struct _GSMCommand GSMCommand;

typedef void (*GSMCommandCallback)(GSM * gsm);


/* functions */
/* commands */
GSMCommand * gsm_command_new(char const * command);
void gsm_command_delete(GSMCommand * gsmc);

/* accessors */
GSMCommandCallback gsm_command_get_callback(GSMCommand * gsmc);
char const * gsm_command_get_command(GSMCommand * gsmc);
GSMError gsm_command_get_error(GSMCommand * gsmc);
GSMMode gsm_command_get_mode(GSMCommand * gsmc);
GSMPriority gsm_command_get_priority(GSMCommand * gsmc);

void gsm_command_set_callback(GSMCommand * gsmc, GSMCommandCallback callback);
void gsm_command_set_error(GSMCommand * gsmc, GSMError error);
void gsm_command_set_mode(GSMCommand * gsmc, GSMMode mode);
void gsm_command_set_priority(GSMCommand * gsmc, GSMPriority priority);

#endif /* !PHONE_COMMAND_H */
