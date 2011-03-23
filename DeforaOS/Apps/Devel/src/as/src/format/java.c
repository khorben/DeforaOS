/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
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



#include <System.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "As/format.h"


/* Java */
/* private */
/* types */
#pragma pack(1)
typedef struct _JavaHeader
{
	uint32_t magic;
	uint16_t minor;
	uint16_t major;
} JavaHeader;
#pragma pack()

typedef struct _JavaPlugin
{
	char * class_name;
	char * super_name;
	uint16_t access_flags;
	uint16_t constants_cnt;
	uint16_t interfaces_cnt;
	uint16_t fields_cnt;
	uint16_t methods_cnt;
	uint16_t attributes_cnt;
} JavaPlugin;


/* prototypes */
static int _java_init(FormatPlugin * format, char const * arch);
static int _java_exit(FormatPlugin * format);


/* public */
/* variables */
/* format_plugin */
FormatPlugin format_plugin =
{
	NULL,
	"\xca\xfe\xba\xbe",
	4,
	_java_init,
	_java_exit,
	NULL,
	NULL,
	NULL
};


/* private */
/* functions */
/* java_init */
static int _java_init(FormatPlugin * format, char const * arch)
{
	JavaHeader jh;
	JavaPlugin * java;

	if(strcmp(arch, "java") != 0)
		return error_set_code(1, "%s: %s", arch,
				"Unsupported architecture for java");
	memcpy(&jh.magic, format->signature, format->signature_len);
	jh.minor = _htob16(0);
	jh.major = _htob16(0x32); /* XXX choose a more appropriate version */
	if(fwrite(&jh, sizeof(jh), 1, format->helper->fp) != 1)
		return error_set_code(1, "%s: %s", format->helper->filename,
				strerror(errno));
	if((java = malloc(sizeof(*java))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	memset(java, 0, sizeof(*java));
	format->priv = java;
	return 0;
}


/* java_exit */
static int _exit_constant_pool(FormatPlugin * format);
static int _exit_access_flags(FormatPlugin * format);
static int _exit_class_name(FormatPlugin * format);
static int _exit_super_name(FormatPlugin * format);
static int _exit_interface_table(FormatPlugin * format);
static int _exit_field_table(FormatPlugin * format);
static int _exit_method_table(FormatPlugin * format);
static int _exit_attribute_table(FormatPlugin * format);

static int _java_exit(FormatPlugin * format)
{
	int ret = 0;

	if(_exit_constant_pool(format) != 0
			|| _exit_access_flags(format) != 0
			|| _exit_class_name(format) != 0
			|| _exit_super_name(format) != 0
			|| _exit_interface_table(format) != 0
			|| _exit_field_table(format) != 0
			|| _exit_method_table(format) != 0
			|| _exit_attribute_table(format) != 0)
		ret = 1;
	free(format->priv);
	return ret;
}

static int _exit_constant_pool(FormatPlugin * format)
{
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->constants_cnt + 1);

	if(fwrite(&cnt, sizeof(cnt), 1, format->helper->fp) != 1)
		return error_set_code(1, "%s: %s", format->helper->filename,
				strerror(errno));
	/* XXX output the constants */
	return 0;
}

static int _exit_access_flags(FormatPlugin * format)
{
	JavaPlugin * java = format->priv;
	uint16_t flags = _htob16(java->access_flags);

	if(fwrite(&flags, sizeof(flags), 1, format->helper->fp) != 1)
		return error_set_code(1, "%s: %s", format->helper->filename,
				strerror(errno));
	return 0;
}

static int _exit_class_name(FormatPlugin * format)
{
	uint16_t index = _htob16(0);

	/* FIXME really implement */
	if(fwrite(&index, sizeof(index), 1, format->helper->fp) != 1)
		return error_set_code(1, "%s: %s", format->helper->filename,
				strerror(errno));
	return 0;
}

static int _exit_super_name(FormatPlugin * format)
{
	uint16_t index = _htob16(0);

	/* FIXME really implement */
	if(fwrite(&index, sizeof(index), 1, format->helper->fp) != 1)
		return error_set_code(1, "%s: %s", format->helper->filename,
				strerror(errno));
	return 0;
}

static int _exit_interface_table(FormatPlugin * format)
{
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->interfaces_cnt);

	if(fwrite(&cnt, sizeof(cnt), 1, format->helper->fp) != 1)
		return error_set_code(1, "%s: %s", format->helper->filename,
				strerror(errno));
	/* XXX output the interfaces */
	return 0;
}

static int _exit_field_table(FormatPlugin * format)
{
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->fields_cnt);

	if(fwrite(&cnt, sizeof(cnt), 1, format->helper->fp) != 1)
		return error_set_code(1, "%s: %s", format->helper->filename,
				strerror(errno));
	/* XXX output the fields */
	return 0;
}

static int _exit_method_table(FormatPlugin * format)
{
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->methods_cnt);

	if(fwrite(&cnt, sizeof(cnt), 1, format->helper->fp) != 1)
		return error_set_code(1, "%s: %s", format->helper->filename,
				strerror(errno));
	/* XXX output the methods */
	return 0;
}

static int _exit_attribute_table(FormatPlugin * format)
{
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->attributes_cnt);

	if(fwrite(&cnt, sizeof(cnt), 1, format->helper->fp) != 1)
		return error_set_code(1, "%s: %s", format->helper->filename,
				strerror(errno));
	/* XXX output the attributes */
	return 0;
}
