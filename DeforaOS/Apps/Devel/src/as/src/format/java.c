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
#include <sys/stat.h>
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
	uint16_t cp_cnt;
} JavaHeader;

typedef enum _JavaCpInfoTag
{
	CONSTANT_Utf8 = 1,
	CONSTANT_Integer = 3,
	CONSTANT_Float = 4,
	CONSTANT_Long = 5,
	CONSTANT_Double = 6,
	CONSTANT_Class = 7,
	CONSTANT_String = 8,
	CONSTANT_Fieldref = 9,
	CONSTANT_Methodref = 10,
	CONSTANT_InterfaceMethodref = 11,
	CONSTANT_NameAndType = 12
} JavaCpInfoTag;

typedef struct _JavaCpInfo
{
	uint8_t tag;
	char info[0];
} JavaCpInfo;

typedef struct _JavaHeader2
{
	uint16_t access;
	uint16_t this;
	uint16_t super;
	uint16_t interfaces_cnt;
} JavaHeader2;
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
/* plug-in */
static int _java_init(FormatPlugin * format, char const * arch);
static int _java_exit(FormatPlugin * format);
static char const * _java_detect(FormatPlugin * format);
static int _java_disas(FormatPlugin * format, int (*callback)(
			FormatPlugin * format, char const * section,
			off_t offset, size_t size, off_t base));

static int _java_error(FormatPlugin * format);


/* public */
/* variables */
/* format_plugin */
FormatPlugin format_plugin =
{
	NULL,
	"java",
	"\xca\xfe\xba\xbe",
	4,
	_java_init,
	_java_exit,
	NULL,
	NULL,
	_java_detect,
	_java_disas,
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
	jh.cp_cnt = _htob16(0);
	if(fwrite(&jh, sizeof(jh), 1, format->helper->fp) != 1)
		return -_java_error(format);
	if((java = malloc(sizeof(*java))) == NULL)
		return -_java_error(format);
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
		return -_java_error(format);
	/* XXX output the constants */
	return 0;
}

static int _exit_access_flags(FormatPlugin * format)
{
	JavaPlugin * java = format->priv;
	uint16_t flags = _htob16(java->access_flags);

	if(fwrite(&flags, sizeof(flags), 1, format->helper->fp) != 1)
		return -_java_error(format);
	return 0;
}

static int _exit_class_name(FormatPlugin * format)
{
	uint16_t index = _htob16(0);

	/* FIXME really implement */
	if(fwrite(&index, sizeof(index), 1, format->helper->fp) != 1)
		return -_java_error(format);
	return 0;
}

static int _exit_super_name(FormatPlugin * format)
{
	uint16_t index = _htob16(0);

	/* FIXME really implement */
	if(fwrite(&index, sizeof(index), 1, format->helper->fp) != 1)
		return -_java_error(format);
	return 0;
}

static int _exit_interface_table(FormatPlugin * format)
{
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->interfaces_cnt);

	if(fwrite(&cnt, sizeof(cnt), 1, format->helper->fp) != 1)
		return -_java_error(format);
	/* XXX output the interfaces */
	return 0;
}

static int _exit_field_table(FormatPlugin * format)
{
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->fields_cnt);

	if(fwrite(&cnt, sizeof(cnt), 1, format->helper->fp) != 1)
		return -_java_error(format);
	/* XXX output the fields */
	return 0;
}

static int _exit_method_table(FormatPlugin * format)
{
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->methods_cnt);

	if(fwrite(&cnt, sizeof(cnt), 1, format->helper->fp) != 1)
		return -_java_error(format);
	/* XXX output the methods */
	return 0;
}

static int _exit_attribute_table(FormatPlugin * format)
{
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->attributes_cnt);

	if(fwrite(&cnt, sizeof(cnt), 1, format->helper->fp) != 1)
		return -_java_error(format);
	/* XXX output the attributes */
	return 0;
}


/* java_detect */
static char const * _java_detect(FormatPlugin * format)
{
	return "java";
}


/* java_disas */
static int _java_disas(FormatPlugin * format, int (*callback)(
			FormatPlugin * format, char const * section,
			off_t offset, size_t size, off_t base))
{
	FILE * fp = format->helper->fp;
	JavaHeader jh;
	size_t i;
	JavaCpInfo jci;
	size_t size;
	char buf[8];
	uint16_t u16;
	JavaHeader2 jh2;
	off_t offset;
	off_t end;

	if(fseek(fp, sizeof(JavaHeader), SEEK_SET) != 0
			|| fread(&jh, sizeof(jh), 1, fp) != 1)
		return -_java_error(format);
	jh.cp_cnt = _htob16(jh.cp_cnt);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() jh.cp_cnt=%u\n", __func__, jh.cp_cnt);
#endif
	/* skip the constant pool */
	for(i = 1; i < jh.cp_cnt; i++)
	{
		if(fread(&jci, sizeof(jci), 1, fp) != 1)
			return -_java_error(format);
		switch(jci.tag)
		{
			case CONSTANT_Double:
			case CONSTANT_Long:
				size = 8;
				break;
			case CONSTANT_Fieldref:
			case CONSTANT_Float:
			case CONSTANT_Integer:
			case CONSTANT_InterfaceMethodref:
			case CONSTANT_NameAndType:
				size = 4;
				break;
			case CONSTANT_Class:
			case CONSTANT_String:
				size = 2;
				break;
			case CONSTANT_Utf8:
				if(fread(&u16, sizeof(u16), 1, fp) != 1)
					return -_java_error(format);
				u16 = _htob16(u16);
				if(fseek(fp, u16, SEEK_CUR) != 0)
					return -_java_error(format);
				size = 0;
				break;
			default:
				return -error_set_code(1, "%s: %s 0x%x",
						format->helper->filename,
						"Unknown constant tag",
						jci.tag);
		}
		if(size != 0 && fread(buf, sizeof(*buf), size, fp) != size)
			return -_java_error(format);
	}
	/* skip the interfaces pool */
	if(fread(&jh2, sizeof(jh2), 1, fp) != 1)
		return -_java_error(format);
	jh2.interfaces_cnt = _htob16(jh2.interfaces_cnt);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() jh2.interfaces_cnt=%u\n", __func__,
			jh2.interfaces_cnt);
#endif
	for(i = 0; i < jh2.interfaces_cnt; i++)
		if(fread(buf, sizeof(*buf), 2, fp) != 2)
			return -_java_error(format);
	if((offset = ftello(fp)) == -1
			|| fseek(fp, 0, SEEK_END) != 0
			|| (end = ftello(fp)) == -1)
		return -_java_error(format);
	return callback(format, NULL, offset, end - offset, 0);
}


/* java_error */
static int _java_error(FormatPlugin * format)
{
	return -error_set_code(1, "%s: %s", format->helper->filename,
			strerror(errno));
}
