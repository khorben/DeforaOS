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
#include <string.h>
#include <errno.h>
#include "As/format.h"


/* DEX */
/* private */
/* types */
#pragma pack(1)
typedef struct _DexHeader
{
	char magic[4];
	char _padding0[4];
	uint32_t checksum;
	unsigned char signature[20];
	uint32_t file_size;		/* 0x20 */
	uint32_t header_size;
	uint32_t endian_tag;
	uint32_t link_size;
	uint32_t link_off;		/* 0x30 */
	uint32_t map_off;
	uint32_t string_ids_size;
	uint32_t string_ids_off;
	uint32_t type_ids_size;		/* 0x40 */
	uint32_t type_ids_off;
	uint32_t proto_ids_size;
	uint32_t proto_ids_off;
	uint32_t fields_ids_size;	/* 0x50 */
	uint32_t fields_ids_off;
	uint32_t method_ids_size;
	uint32_t method_ids_off;
	uint32_t class_defs_size;	/* 0x60 */
	uint32_t class_defs_off;
	uint32_t data_size;
	uint32_t data_off;
} DexHeader;

enum
{
	TYPE_HEADER_ITEM	= 0x0000,
	TYPE_CODE_ITEM		= 0x2001
};

typedef struct _DexMapItem
{
	uint16_t type;
	uint16_t _padding;
	uint32_t size;
	uint32_t offset;
} DexMapItem;
#pragma pack()


/* variables */
static char _dex_signature[4] = "dex\n";


/* prototypes */
/* plug-in */
static char const * _dex_detect(FormatPlugin * format);
static int _dex_disas(FormatPlugin * format, int (*callback)(
			FormatPlugin * format, char const * section,
			off_t offset, size_t size, off_t base));

static int _dex_error(FormatPlugin * format);
static int _dex_error_fread(FormatPlugin * format);


/* public */
/* variables */
FormatPlugin format_plugin =
{
	NULL,
	"dex",
	_dex_signature,
	sizeof(_dex_signature),
	NULL,
	NULL,
	NULL,
	NULL,
	_dex_detect,
	_dex_disas,
	NULL
};


/* private */
/* functions */
/* plug-in */
/* dex_detect */
static char const * _dex_detect(FormatPlugin * format)
{
	/* FIXME some sections might contain native code */
	return "dalvik";
}


/* dex_disas */
static int _disas_map(FormatPlugin * format, DexHeader * dh, int (*callback)(
			FormatPlugin * format, char const * section,
			off_t offset, size_t size, off_t base));

static int _dex_disas(FormatPlugin * format, int (*callback)(
			FormatPlugin * format, char const * section,
			off_t offset, size_t size, off_t base))
{
	FILE * fp = format->helper->fp;
	DexHeader dh;
	off_t offset;
	off_t end;

	if(fseek(fp, 0, SEEK_SET) != 0)
		return -_dex_error(format);
	if(fread(&dh, sizeof(dh), 1, fp) != 1)
		return _dex_error_fread(format);
	/* FIXME implement endian */
	if(_disas_map(format, &dh, callback) != 0)
		return -1;
	return 0;
}

static int _disas_map(FormatPlugin * format, DexHeader * dh, int (*callback)(
			FormatPlugin * format, char const * section,
			off_t offset, size_t size, off_t base))
{
	FILE * fp = format->helper->fp;
	uint32_t size;
	uint32_t i;
	fpos_t pos;
	DexMapItem dmi;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(fseek(fp, dh->map_off, SEEK_SET) != 0)
		return -_dex_error(format);
	if(fread(&size, sizeof(size), 1, fp) != 1)
		return -_dex_error_fread(format);
	size = _htol32(size);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u items\n", __func__, size);
#endif
	for(i = 0; i < size; i++)
	{
		if(fread(&dmi, sizeof(dmi), 1, fp) != 1)
			return -_dex_error_fread(format);
		fgetpos(fp, &pos);
		dmi.type = _htol16(dmi.type);
		dmi.size = _htol32(dmi.size);
		dmi.offset = _htol32(dmi.offset);
		switch(dmi.type)
		{
			case TYPE_CODE_ITEM:
				callback(format, NULL, dmi.offset, dmi.size,
						0);
				break;
		}
		fsetpos(fp, &pos);
	}
	return 0;
}


/* dex_error */
static int _dex_error(FormatPlugin * format)
{
	return -error_set_code(1, "%s: %s", format->helper->filename,
			strerror(errno));
}


/* dex_error_fread */
static int _dex_error_fread(FormatPlugin * format)
{
	if(ferror(format->helper->fp))
		return _dex_error(format);
	return -error_set_code(1, "%s: %s", format->helper->filename,
			"End of file reached");
}
