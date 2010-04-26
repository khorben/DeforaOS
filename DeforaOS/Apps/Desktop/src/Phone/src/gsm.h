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



#ifndef PHONE_GSM_H
# define PHONE_GSM_H


/* GSM */
/* types */
typedef enum _GSMCallType
{
	GSM_CALL_TYPE_DATA, GSM_CALL_TYPE_VOICE
} GSMCallType;

typedef struct _GSM GSM;


/* functions */
GSM * gsm_new(char const * device, unsigned int baudrate);
void gsm_delete(GSM * gsm);

/* accessors */
unsigned int gsm_get_retry(GSM * gsm);
void gsm_set_retry(GSM * gsm, unsigned int retry);

/* useful */
int gsm_call(GSM * gsm, GSMCallType calltype, char const * number);
int gsm_hangup(GSM * gsm);
int gsm_report_signal_quality(GSM * gsm);
void gsm_reset(GSM * gsm, unsigned int delay);

#endif /* !PHONE_GSM_H */
