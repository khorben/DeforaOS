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



#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <errno.h>
#include "command.h"


/* GSMCommand */
/* private */
/* types */
struct _GSMCommand
{
	GSMPriority priority;
	char * command;
	unsigned int timeout;
	GSMError error;
	GSMCommandCallback callback;
	GSMMode mode;
};


/* public */
/* functions */
/* gsm_command_new */
GSMCommand * gsm_command_new(char const * command)
{
	GSMCommand * gsmc;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, command);
#endif
	if((gsmc = malloc(sizeof(*gsmc))) == NULL)
		return NULL; /* XXX report error */
	gsmc->priority = GSM_PRIORITY_NORMAL;
	gsmc->command = strdup(command);
	gsmc->timeout = 2000;
	gsmc->error = GSM_ERROR_UNKNOWN;
	gsmc->callback = NULL;
	gsmc->mode = GSM_MODE_COMMAND;
	/* check errors */
	if(gsmc->command == NULL)
	{
		gsm_command_delete(gsmc);
		return NULL;
	}
	return gsmc;
}


/* gsm_command_delete */
void gsm_command_delete(GSMCommand * gsmc)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	free(gsmc->command);
	free(gsmc);
}


/* accessors */
/* gsm_command_get_callback */
GSMCommandCallback gsm_command_get_callback(GSMCommand * gsmc)
{
	return gsmc->callback;
}


/* gsm_command_get_command */
char const * gsm_command_get_command(GSMCommand * gsmc)
{
	return gsmc->command;
}


/* gsm_command_get_error */
GSMError gsm_command_get_error(GSMCommand * gsmc)
{
	return gsmc->error;
}


/* gsm_command_get_mode */
GSMMode gsm_command_get_mode(GSMCommand * gsmc)
{
	return gsmc->mode;
}


/* gsm_command_get_priority */
GSMPriority gsm_command_get_priority(GSMCommand * gsmc)
{
	return gsmc->priority;
}


/* gsm_command_get_timeout */
unsigned int gsm_command_get_timeout(GSMCommand * gsmc)
{
	return gsmc->timeout;
}


/* gsm_command_set_callback */
void gsm_command_set_callback(GSMCommand * gsmc,
		GSMCommandCallback callback)
{
	gsmc->callback = callback;
}


/* gsm_command_set_error */
void gsm_command_set_error(GSMCommand * gsmc, GSMError error)
{
	gsmc->error = error;
}


/* gsm_command_set_mode */
void gsm_command_set_mode(GSMCommand * gsmc, GSMMode mode)
{
	gsmc->mode = mode;
}


/* gsm_command_set_priority */
void gsm_command_set_priority(GSMCommand * gsmc, GSMPriority priority)
{
	gsmc->priority = priority;
}


/* gsm_command_set_timeout */
void gsm_command_set_timeout(GSMCommand * gsmc, unsigned int timeout)
{
	gsmc->timeout = timeout;
}
