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
#include <stdio.h>
#include <string.h>
#include <System.h>
#include "../src/plugins/smscrypt.c"


/* helper_config_get */
static char const * _helper_config_get(Phone * phone, char const * section,
		char const * variable)
{
	Config * config = (Config *)phone;

	return config_get(config, section, variable);
}


/* hexdump */
static void _hexdump(char const * buf, size_t len)
{
	unsigned char const * b = (unsigned char *)buf;
	size_t i;

	for(i = 0; i < len; i++)
	{
		printf(" %02x", b[i]);
		if((i % 16) == 7)
			fputc(' ', stdout);
		else if((i % 16) == 15)
			fputc('\n', stdout);
	}
	fputc('\n', stdout);
}


/* main */
int main(int argc, char * argv[])
{
	PhonePluginHelper helper;
	Config * config;
	PhoneEncoding encoding = PHONE_ENCODING_UTF8;
	char * p;
	size_t len;

	if(argc != 2)
		return 1;
	config = config_new();
	config_load(config, "/home/khorben/.phone"); /* FIXME hardcoded */
	helper.phone = (Phone *)config;
	helper.config_get = _helper_config_get;
	plugin.helper = &helper;
	if(_smscrypt_init(&plugin) != 0)
	{
		error_print("smscrypt");
		return 2;
	}
	printf("Message: \"%s\"\n", argv[1]);
	if((p = strdup(argv[1])) == NULL)
		return 2;
	len = strlen(p);
	_smscrypt_event(&plugin, PHONE_EVENT_SMS_SENDING, NULL, &encoding, &p,
			&len);
	printf("Encrypted:\n");
	_hexdump(p, len);
	_smscrypt_event(&plugin, PHONE_EVENT_SMS_RECEIVING, NULL,
			&encoding, &p, &len);
	printf("Message: \"%s\"\n", p);
	free(p);
	_smscrypt_destroy(&plugin);
	config_delete(config);
	return 0;
}
