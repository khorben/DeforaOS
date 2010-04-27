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

# include <glib.h>


/* Phone */
/* types */
typedef struct _Phone Phone;

typedef enum _PhoneCode
{
	PHONE_CODE_SIM_PIN = 0
} PhoneCode;


/* functions */
Phone * phone_new(char const * device, unsigned int baudrate, int retry);
void phone_delete(Phone * phone);


/* useful */
int phone_error(Phone * phone, char const * message, int ret);

/* interface */
void phone_show_code(Phone * phone, gboolean show);
void phone_show_contacts(Phone * phone, gboolean show);
void phone_show_dialer(Phone * phone, gboolean show);
void phone_show_messages(Phone * phone, gboolean show);

/* code */
void phone_code_append(Phone * phone, char character);
void phone_code_enter(Phone * phone, PhoneCode code);
void phone_code_validate(Phone * phone);

/* contacts */
void phone_contact_add(Phone * phone, unsigned int index, char const * name,
		char const * number);

/* dialer */
void phone_call(Phone * phone, char const * number);
void phone_hangup(Phone * phone);

void phone_dialer_append(Phone * phone, char character);

#endif /* !PHONE_PHONE_H */
