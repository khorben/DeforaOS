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



#define DEBUG
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <System.h>
#include "../src/plugins/smscrypt.c"


/* helper_config_foreach */
static void _helper_config_foreach(Phone * phone, char const * section,
		PhoneConfigForeachCallback callback, void * priv)
{
	Config * config = (Config*)phone;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\", %p, %p)\n", __func__,
			(void*)phone, section, (void*)callback, priv);
#endif
	config_foreach_section(config, section, callback, priv);
}


/* helper_config_get */
static char const * _helper_config_get(Phone * phone, char const * section,
		char const * variable)
{
	Config * config = (Config*)phone;
#ifdef DEBUG
	char const * ret;

	fprintf(stderr, "DEBUG: %s(%p, \"%s\", \"%s\")\n", __func__,
			(void*)phone, section, variable);
	ret = config_get(config, section, variable);
	fprintf(stderr, "DEBUG: %s() => \"%s\"\n", __func__, ret);
	return ret;
#else
	return config_get(config, section, variable);
#endif
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


/* usage */
static int _usage(void)
{
	fputs("Usage: smscrypt [-p number] message\n", stderr);
	return 1;
}


/* main */
static gboolean _main_idle(gpointer data);

int main(int argc, char * argv[])
{
	int o;
	struct { char const * message; char const * number; } mn;
	
	mn.number = NULL;
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "p:")) != -1)
		switch(o)
		{
			case 'p':
				mn.number = optarg;
				break;
			default:
				return _usage();
		}
	if(optind + 1 != argc)
		return _usage();
	mn.message = argv[optind];
	g_idle_add(_main_idle, &mn);
	gtk_main();
	return 0;
}

static gboolean _main_idle(gpointer data)
{
	struct { char const * message; char const * number; } * mn = data;
	PhonePluginHelper helper;
	Config * config;
	PhoneEncoding encoding = PHONE_ENCODING_UTF8;
	char * p;
	size_t len;

	config = config_new();
	config_load(config, "/home/khorben/.phone"); /* FIXME hardcoded */
	helper.phone = (Phone *)config;
	helper.config_foreach = _helper_config_foreach;
	helper.config_get = _helper_config_get;
	plugin.helper = &helper;
	if(_smscrypt_init(&plugin) != 0)
	{
		error_print("smscrypt");
		return FALSE;
	}
	if((p = strdup(mn->message)) == NULL)
		return FALSE;
	printf("Message: \"%s\"\n", p);
	len = strlen(p);
	if(_smscrypt_event(&plugin, PHONE_EVENT_SMS_SENDING, mn->number,
				&encoding, &p, &len) != 0)
		puts("Could not encrypt");
	else
	{
		printf("Encrypted:\n");
		_hexdump(p, len);
		if(_smscrypt_event(&plugin, PHONE_EVENT_SMS_RECEIVING,
					mn->number, &encoding, &p, &len) != 0)
			puts("Could not decrypt");
		else
			printf("Decrypted: \"%s\"\n", p);
	}
	free(p);
	_smscrypt_destroy(&plugin);
	config_delete(config);
	gtk_main_quit();
	return FALSE;
}
