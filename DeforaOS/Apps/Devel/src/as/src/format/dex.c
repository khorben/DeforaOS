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


/* Flat */
/* private */
/* types */
#pragma pack(1)
typedef struct _DexHeader
{
	char signature[4];
	char _padding0[4];
	uint32_t checksum;
	char sha1sum[20];
	uint32_t size;			/* 0x20 */
	uint32_t hsize;
	uint32_t endian;
	char _padding1[4];
	char _padding2[4];		/* 0x30 */
	char _padding3[4];
	uint32_t strings_cnt;
	uint32_t strings_offset;
	char _padding4[4];		/* 0x40 */
	uint32_t classes_cnt;
	uint32_t classes_offset;
	uint32_t fields_cnt;
	uint32_t fields_offset;
	uint32_t methods_cnt;		/* 0x50 */
	uint32_t methods_offset;
	uint32_t classdefs_cnt;
	uint32_t classdefs_offset;
} DexHeader;

typedef struct _DexMethod
{
	uint32_t class;
	uint32_t name;
	uint32_t type;
} DexMethod;
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
	return "java";
}


/* dex_disas */
static int _dex_disas(FormatPlugin * format, int (*callback)(
			FormatPlugin * format, char const * section,
			off_t offset, size_t size, off_t base))
{
	FILE * fp = format->helper->fp;
	DexHeader dh;
	size_t i;
	DexMethod dm;
	off_t offset;
	off_t end;

	if(fseek(fp, 0, SEEK_SET) != 0)
		return -_dex_error(format);
	if(fread(&dh, sizeof(dh), 1, fp) != 1)
		return _dex_error_fread(format);
	dh.strings_cnt = _htol32(dh.strings_cnt);
	dh.strings_offset = _htol32(dh.strings_offset);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() cnt=0x%x offset=0x%x\n", __func__,
			dh.strings_cnt, dh.strings_offset);
#endif
	dh.methods_cnt = _htol32(dh.methods_cnt);
	dh.methods_offset = _htol32(dh.methods_offset);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() cnt=0x%x offset=0x%x\n", __func__,
			dh.methods_cnt, dh.methods_offset);
#endif
	if(fseek(fp, dh.methods_offset, SEEK_SET) != 0)
		return -_dex_error(format);
	for(i = 0; i < dh.methods_cnt; i++)
		if(fread(&dm, sizeof(dm), 1, fp) != 1)
			return _dex_error_fread(format);
	/* FIXME really implement */
	/* disassemble the rest */
	if((offset = ftello(fp)) == -1
			|| fseek(fp, 0, SEEK_END) != 0
			|| (end = ftello(fp)) == -1)
		return -_dex_error(format);
	return callback(format, NULL, offset, end - offset, 0);
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
