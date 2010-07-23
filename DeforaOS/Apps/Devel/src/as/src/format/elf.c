/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
/* as is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * as is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with as; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <System.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <elf.h>
#include "format.h"
#include "../../config.h"


/* ELF */
/* private */
/* types */
typedef struct _ElfArch
{
	char const * arch;
	unsigned char machine;
	unsigned char capacity;
	unsigned char endian;
} ElfArch;

typedef struct _ElfSectionValues
{
	char const * name;
	Elf32_Word type;	/* works for 64-bit too */
	Elf32_Word flags;
} ElfSectionValues;

typedef struct _ElfStrtab
{
	char * buf;
	size_t cnt;
} ElfStrtab;


/* prototypes */
/* ELF */
static int _elf_init(FormatPlugin * format, char const * arch);

/* ELF32 */
static int _init_32(FILE * fp);
static int _exit_32(FormatPlugin * format);
static int _section_32(FormatPlugin * format, char const * name);

/* ELF64 */
static int _init_64(FILE * fp);
static int _exit_64(FormatPlugin * format);
static int _section_64(FormatPlugin * format, char const * name);

/* ElfStrtab */
static int _elfstrtab_set(ElfStrtab * strtab, char const * name);


/* variables */
static ElfArch elf_arch[] =
{
	{ "amd64",	EM_X86_64,	ELFCLASS64,	ELFDATA2LSB	},
	{ "i386",	EM_386,		ELFCLASS32,	ELFDATA2LSB	},
	{ "i486",	EM_386,		ELFCLASS32,	ELFDATA2LSB	},
	{ "i586",	EM_386,		ELFCLASS32,	ELFDATA2LSB	},
	{ "i686",	EM_386,		ELFCLASS32,	ELFDATA2LSB	},
	{ "sparc",	EM_SPARC,	ELFCLASS32,	ELFDATA2MSB	},
	{ "sparc64",	EM_SPARCV9,	ELFCLASS64,	ELFDATA2MSB	},
	{ NULL,		'\0',		'\0',		'\0'		}
};
static ElfArch * ea;

static ElfSectionValues elf_section_values[] =
{
	{ ".bss",	SHT_NOBITS,	SHF_ALLOC | SHF_WRITE		},
	{ ".comment",	SHT_PROGBITS,	0				},
	{ ".data",	SHT_PROGBITS,	SHF_ALLOC | SHF_WRITE		},
	{ ".data1",	SHT_PROGBITS,	SHF_ALLOC | SHF_WRITE		},
	{ ".debug",	SHT_PROGBITS,	0				},
	{ ".dynamic",	SHT_DYNAMIC,	0				},
	{ ".dynstr",	SHT_STRTAB,	SHF_ALLOC			},
	{ ".dynsym",	SHT_DYNSYM,	SHF_ALLOC			},
	{ ".fini",	SHT_PROGBITS,	SHF_ALLOC | SHF_EXECINSTR	},
	{ ".got",	SHT_PROGBITS,	0				},
	{ ".hash",	SHT_HASH,	SHF_ALLOC			},
	{ ".init",	SHT_PROGBITS,	SHF_ALLOC | SHF_EXECINSTR	},
	{ ".interp",	SHT_PROGBITS,	0				},
	{ ".line",	SHT_PROGBITS,	0				},
	{ ".note",	SHT_NOTE,	0				},
	{ ".plt",	SHT_PROGBITS,	0				},
	{ ".rodata",	SHT_PROGBITS,	SHF_ALLOC			},
	{ ".rodata1",	SHT_PROGBITS,	SHF_ALLOC			},
	{ ".shstrtab",	SHT_STRTAB,	0				},
	{ ".strtab",	SHT_STRTAB,	0				},
	{ ".symtab",	SHT_SYMTAB,	0				},
	{ ".text",	SHT_PROGBITS,	SHF_ALLOC | SHF_EXECINSTR	},
	{ NULL,		0,		0				}
};

static ElfStrtab shstrtab = { NULL, 0 };	/* section string table */


/* public */
/* variables */
/* format_plugin */
FormatPlugin format_plugin =
{
	NULL,
	NULL,
	_elf_init,
	NULL,
	NULL,
	NULL,
	NULL
};


/* private */
/* functions */
/* elf_error */
static int _elf_error(char const * message, int ret)
{
	error_set_code(ret, "%s: %s", message, strerror(errno));
	return ret;
}


/* elf_init */
static ElfArch * _init_arch(char const * arch);

static int _elf_init(FormatPlugin * format, char const * arch)
{
	if((ea = _init_arch(arch)) == NULL)
		return 1;
	if(ea->capacity == ELFCLASS32)
	{
		if(_init_32(format->fp) != 0)
			return 1;
		format->exit = _exit_32;
		format->section = _section_32;
	}
	else if(ea->capacity == ELFCLASS64)
	{
		if(_init_64(format->fp) != 0)
			return 1;
		format->exit = _exit_64;
		format->section = _section_64;
	}
	else
		return 1;
	return 0;
}

static ElfArch * _init_arch(char const * arch)
{
	ElfArch * ea;

	for(ea = elf_arch; ea->arch != NULL; ea++)
		if(strcmp(ea->arch, arch) == 0)
			return ea;
	error_set_code(1, "%s: %s", arch, "Unsupported ELF architecture");
	return NULL;
}


/* section_values */
static ElfSectionValues * _section_values(char const * name)
{
	ElfSectionValues * esv;
	int cmp;

	for(esv = elf_section_values; esv->name != NULL; esv++)
		if((cmp = strcmp(esv->name, name)) == 0)
			return esv;
		else if(cmp > 0)
			break;
	for(; esv->name != NULL; esv++);
	return esv;
}


/* ELF32 */
/* variables */
static Elf32_Shdr * es32 = NULL;
static int es32_cnt = 0;


/* init_32 */
static int _init_32(FILE * fp)
{
	Elf32_Ehdr hdr;

	memset(&hdr, 0, sizeof(hdr));
	memcpy(&hdr.e_ident, ELFMAG, SELFMAG);
	hdr.e_ident[EI_CLASS] = ELFCLASS32;
	hdr.e_ident[EI_DATA] = ea->endian;
	hdr.e_ident[EI_VERSION] = EV_CURRENT;
	if(ea->endian == ELFDATA2MSB)
	{
		hdr.e_type = _htob16(ET_REL);
		hdr.e_machine = _htob16(ea->machine);
		hdr.e_version = _htob32(EV_CURRENT);
		hdr.e_ehsize = _htob16(sizeof(hdr));
		hdr.e_shentsize = _htob16(sizeof(Elf32_Shdr));
		hdr.e_shstrndx = _htob16(SHN_UNDEF);
	}
	else
	{
		hdr.e_type = _htol16(ET_REL);
		hdr.e_machine = _htol16(ea->machine);
		hdr.e_version = _htol32(EV_CURRENT);
		hdr.e_ehsize = _htol16(sizeof(hdr));
		hdr.e_shentsize = _htol16(sizeof(Elf32_Shdr));
		hdr.e_shstrndx = _htol16(SHN_UNDEF);
	}
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error(format_plugin.filename, 1);
	return 0;
}


/* exit_32 */
static int _exit_32_phdr(FormatPlugin * format, Elf32_Off offset);
static int _exit_32_shdr(FormatPlugin * format, Elf32_Off offset);

static int _exit_32(FormatPlugin * format)
{
	int ret = 0;
	long offset;

	if(_section_32(format, ".shstrtab") != 0)
		ret = 1;
	else if(fwrite(shstrtab.buf, sizeof(char), shstrtab.cnt, format->fp)
			!= shstrtab.cnt)
		ret = _elf_error(format->filename, 1);
	else if((offset = ftell(format->fp)) == -1)
		ret = _elf_error(format->filename, 1);
	else if(_exit_32_phdr(format, offset) != 0
			|| _exit_32_shdr(format, offset) != 0)
		ret = 1;
	free(es32);
	es32 = NULL;
	es32_cnt = 0;
	free(shstrtab.buf);
	shstrtab.buf = NULL;
	shstrtab.cnt = 0;
	return ret;
}

static int _exit_32_phdr(FormatPlugin * format, Elf32_Off offset)
{
	Elf32_Ehdr hdr;

	if(es32_cnt == 0)
		return 0;
	if(fseek(format->fp, 0, SEEK_SET) != 0)
		return _elf_error(format->filename, 1);
	if(fread(&hdr, sizeof(hdr), 1, format->fp) != 1)
		return _elf_error(format->filename, 1);
	if(ea->endian == ELFDATA2MSB)
	{
		hdr.e_shoff = _htob32(offset);
		hdr.e_shnum = _htob16(es32_cnt + 1);
		hdr.e_shstrndx = _htob16(es32_cnt);
	}
	else
	{
		hdr.e_shoff = _htol32(offset);
		hdr.e_shnum = _htol16(es32_cnt + 1);
		hdr.e_shstrndx = _htol16(es32_cnt);
	}
	if(fseek(format->fp, 0, SEEK_SET) != 0)
		return _elf_error(format->filename, 1);
	if(fwrite(&hdr, sizeof(hdr), 1, format->fp) != 1)
		return _elf_error(format->filename, 1);
	return 0;
}

static int _exit_32_shdr(FormatPlugin * format, Elf32_Off offset)
{
	Elf32_Shdr hdr;
	int i;

	if(fseek(format->fp, 0, SEEK_END) != 0)
		return _elf_error(format->filename, 1);
	memset(&hdr, 0, sizeof(hdr));
	if(ea->endian == ELFDATA2MSB)
	{
		hdr.sh_type = _htob32(SHT_NULL);
		hdr.sh_link = _htob32(SHN_UNDEF);
	}
	else
	{
		hdr.sh_type = _htol32(SHT_NULL);
		hdr.sh_link = _htol32(SHN_UNDEF);
	}
	if(fwrite(&hdr, sizeof(hdr), 1, format->fp) != 1)
		return _elf_error(format->filename, 1);
	for(i = 0; i < es32_cnt; i++)
	{
		if(i+1 == es32_cnt)
			es32[i].sh_size = offset - es32[i].sh_offset;
		else
			es32[i].sh_size = es32[i + 1].sh_offset
				- es32[i].sh_offset;
		es32[i].sh_size = ea->endian == ELFDATA2MSB
			? _htob32(es32[i].sh_size) : _htol32(es32[i].sh_size);
		if(fwrite(&es32[i], sizeof(Elf32_Shdr), 1, format->fp) != 1)
			return _elf_error(format->filename, 1);
	}
	return 0;
}


/* section_32 */
static int _section_32(FormatPlugin * format, char const * name)
{
	int ss;
	Elf32_Shdr * p;
	ElfSectionValues * esv;
	long offset;

	if((ss = _elfstrtab_set(&shstrtab, name)) < 0)
		return 1;
	if((p = realloc(es32, sizeof(*es32) * (es32_cnt + 1))) == NULL)
		return _elf_error(format->filename, 1);
	es32 = p;
	p = &es32[es32_cnt++];
	memset(p, 0, sizeof(*p));
	esv = _section_values(name);
	p->sh_name = ss;
	p->sh_type = esv->type;
	p->sh_flags = esv->flags;
	if((offset = ftell(format->fp)) == -1)
		return _elf_error(format->filename, 1);
	p->sh_offset = offset;
	p->sh_link = SHN_UNDEF; /* FIXME */
	return 0;
}


/* ELF64 */
/* variables */
static Elf64_Shdr * es64 = NULL;
static int es64_cnt = 0;


/* init_64 */
static int _init_64(FILE * fp)
{
	Elf64_Ehdr hdr;

	memset(&hdr, 0, sizeof(hdr));
	memcpy(&hdr.e_ident, ELFMAG, SELFMAG);
	hdr.e_ident[EI_CLASS] = ELFCLASS64;
	hdr.e_ident[EI_DATA] = ea->endian;
	hdr.e_ident[EI_VERSION] = EV_CURRENT;
	if(ea->endian == ELFDATA2MSB)
	{
		hdr.e_type = _htob16(ET_REL);
		hdr.e_machine = _htob16(ea->machine);
		hdr.e_version = _htob32(EV_CURRENT);
		hdr.e_ehsize = _htob16(sizeof(hdr));
		hdr.e_shentsize = _htob16(sizeof(Elf64_Shdr));
	}
	else
	{
		hdr.e_type = _htol16(ET_REL);
		hdr.e_machine = _htol16(ea->machine);
		hdr.e_version = _htol32(EV_CURRENT);
		hdr.e_ehsize = _htol16(sizeof(hdr));
		hdr.e_shentsize = _htol16(sizeof(Elf64_Shdr));
	}
	hdr.e_shstrndx = SHN_UNDEF;
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error(format_plugin.filename, 1);
	return 0;
}


/* exit_64 */
static int _exit_64_phdr(FormatPlugin * format, Elf64_Off offset);
static int _exit_64_shdr(FormatPlugin * format, Elf64_Off offset);

static int _exit_64(FormatPlugin * format)
{
	int ret = 0;
	long offset;

	if(_section_64(format, ".shstrtab") != 0)
		ret = 1;
	else if(fwrite(shstrtab.buf, sizeof(char), shstrtab.cnt, format->fp)
			!= shstrtab.cnt)
		ret = _elf_error(format_plugin.filename, 1);
	else if((offset = ftell(format->fp)) == -1)
		ret = _elf_error(format_plugin.filename, 1);
	else if(_exit_64_phdr(format, offset) != 0
			|| _exit_64_shdr(format, offset) != 0)
		ret = 1;
	free(es64);
	es64 = NULL;
	es64_cnt = 0;
	free(shstrtab.buf);
	shstrtab.buf = NULL;
	shstrtab.cnt = 0;
	return ret;
}

static int _exit_64_phdr(FormatPlugin * format, Elf64_Off offset)
{
	Elf64_Ehdr hdr;

	if(es64_cnt == 0)
		return 0;
	if(fseek(format->fp, 0, SEEK_SET) != 0)
		return _elf_error(format->filename, 1);
	if(fread(&hdr, sizeof(hdr), 1, format->fp) != 1)
		return _elf_error(format->filename, 1);
	if(ea->endian == ELFDATA2MSB)
	{
		hdr.e_shoff = _htob64(offset);
		hdr.e_shnum = _htob16(es64_cnt);
		hdr.e_shstrndx = _htob16(es64_cnt - 1);
	}
	else
	{
		hdr.e_shoff = _htol64(offset);
		hdr.e_shnum = _htol16(es64_cnt);
		hdr.e_shstrndx = _htol16(es64_cnt-1);
	}
	if(fseek(format->fp, 0, SEEK_SET) != 0)
		return _elf_error(format->filename, 1);
	if(fwrite(&hdr, sizeof(hdr), 1, format->fp) != 1)
		return _elf_error(format->filename, 1);
	return 0;
}

static int _exit_64_shdr(FormatPlugin * format, Elf64_Off offset)
{
	int i;

	if(fseek(format->fp, 0, SEEK_END) != 0)
		return _elf_error(format->filename, 1);
	for(i = 0; i < es64_cnt; i++)
	{
		if(i + 1 == es64_cnt)
			es64[i].sh_size = offset - es64[i].sh_offset;
		else
			es64[i].sh_size = es64[i+1].sh_offset
				- es64[i].sh_offset;
		es64[i].sh_size = ea->endian == ELFDATA2MSB
			? _htob64(es64[i].sh_size) : _htol64(es64[i].sh_size);
		if(fwrite(&es64[i], sizeof(Elf64_Shdr), 1, format->fp) != 1)
			return _elf_error(format->filename, 1);
	}
	return 0;
}


/* section_64 */
static int _section_64(FormatPlugin * format, char const * name)
{
	int ss;
	Elf64_Shdr * p;
	ElfSectionValues * esv;
	long offset;

	if((ss = _elfstrtab_set(&shstrtab, name)) < 0)
		return 1;
	if((p = realloc(es64, sizeof(*es64) * (es64_cnt + 1))) == NULL)
		return _elf_error(format->filename, 1);
	es64 = p;
	p = &es64[es64_cnt++];
	memset(p, 0, sizeof(*p));
	esv = _section_values(name);
	p->sh_name = ss;
	p->sh_type = esv->type;
	p->sh_flags = esv->flags;
	if((offset = ftell(format->fp)) == -1)
		return _elf_error(format->filename, 1);
	p->sh_offset = offset;
	p->sh_link = SHN_UNDEF; /* FIXME */
	return 0;
}


/* ElfStrtab */
/* private */
/* functions */
/* elfstrtab_get */
static int _elfstrtab_set(ElfStrtab * strtab, char const * name)
{
	size_t len;
	size_t cnt;
	char * p;

	if((len = strlen(name)) == 0 && strtab->cnt != 0)
		return 0;
	if((cnt = strtab->cnt) == 0)
		cnt++;
	if((p = realloc(strtab->buf, sizeof(char) * (cnt + len + 1))) == NULL)
		return -_elf_error(format_plugin.filename, 1);
	else if(strtab->buf == NULL)
		p[0] = '\0';
	strtab->buf = p;
	if(len == 0)
	{
		strtab->cnt = cnt;
		return 0;
	}
	strtab->cnt = cnt + len + 1;
	memcpy(&strtab->buf[cnt], name, len + 1);
	return cnt;
}
