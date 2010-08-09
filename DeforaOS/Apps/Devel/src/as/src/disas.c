/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include "as.h"
#include "arch/arch.h"


/* disas */
/* private */
/* types */
typedef struct _Disas
{
	char const * filename;
	FILE * fp;
	As * as;
	Arch * arch;
} Disas;

typedef struct _DisasSignature
{
	char const * name;
	char const * signature;
	size_t size;
	int (*callback)(Disas * disas);
} DisasSignature;


/* variables */
static int _disas_elf(Disas * disas);
static int _disas_flat(Disas * disas);

static DisasSignature _disas_signatures[] =
{
	{ "elf",	ELFMAG, SELFMAG,	_disas_elf	},
	{ "flat",	NULL, 0,		_disas_flat	}
};
#define _disas_signatures_cnt (sizeof(_disas_signatures) \
		/ sizeof(*_disas_signatures))


/* prototypes */
static int _disas(char const * arch, char const * format,
		char const * filename);
static int _disas_list(void);

static int _disas_error(char const * message, int ret);


/* functions */
/* disas */
static int _disas_do_format(Disas * disas, char const * format);
static int _disas_do(Disas * disas);
static int _do_callback(Disas * disas, size_t i);
static int _do_flat(Disas * disas, off_t offset, size_t size);
static int _do_flat_print(Disas * disas, ArchInstruction * ai);

static int _disas(char const * arch, char const * format, char const * filename)
{
	int ret = 1;
	Disas disas;

	if((disas.as = as_new(arch, NULL)) == NULL)
		return error_print("disas");
	if((disas.fp = fopen(filename, "r")) == NULL)
		ret = _disas_error(filename, 1);
	else
	{
		disas.filename = filename;
		ret = _disas_do_format(&disas, format);
		fclose(disas.fp);
	}
	as_delete(disas.as);
	return ret;
}

static int _disas_do_format(Disas * disas, char const * format)
{
	size_t i;

	if(format == NULL)
		return _disas_do(disas);
	for(i = 0; i < _disas_signatures_cnt; i++)
		if(strcmp(_disas_signatures[i].name, format) == 0)
			return _do_callback(disas, i);
	fprintf(stderr, "disas: %s: %s\n", format, "Unknown format");
	return 1;
}

static int _disas_do(Disas * disas)
{
	int ret = 1;
	size_t i;
	size_t s = 0;
	char * buf;

	for(i = 0; i < _disas_signatures_cnt; i++)
		if(_disas_signatures[i].size > s)
			s = _disas_signatures[i].size;
	if((buf = malloc(s)) == NULL
			|| fread(buf, sizeof(*buf), s, disas->fp) != s)
	{
		free(buf);
		return _disas_error(disas->filename, 1);
	}
	for(i = 0; i < _disas_signatures_cnt; i++)
		if(memcmp(_disas_signatures[i].signature, buf,
					_disas_signatures[i].size) == 0)
		{
			ret = _do_callback(disas, i);
			break;
		}
	free(buf);
	return ret;
}

static int _do_callback(Disas * disas, size_t i)
{
	printf("\n%s: %s-%s\n", disas->filename, _disas_signatures[i].name,
			as_get_arch(disas->as));
	return _disas_signatures[i].callback(disas);
}

static int _do_flat(Disas * disas, off_t offset, size_t size)
{
	int ret = 0;
	size_t i;
	unsigned int opcode;
	size_t j;
	int c;
	ArchInstruction * ai;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, 0x%lx, 0x%lx) arch=\"%s\"\n", __func__,
			disas, (unsigned long)offset, (unsigned long)size,
			as_get_arch(disas->as));
#endif
	if(fseek(disas->fp, offset, SEEK_SET) != 0)
		return _disas_error(disas->filename, 1);
	if((disas->arch = arch_new(as_get_arch(disas->as))) == NULL)
		return error_print("disas");
	printf("\n%08lx:\n", (unsigned long)offset);
	for(i = 0; i < size; i++)
	{
		printf("%5zx: ", i);
		opcode = 0;
		ret = 0;
		for(j = 0; j < 4; j++)
		{
			if((c = fgetc(disas->fp)) == EOF)
				break;
			printf(" %02x", c);
			opcode = (opcode << 8) | c;
			if((ai = arch_instruction_get_by_opcode(disas->arch,
							j + 1, opcode)) == NULL)
				continue;
			ret = _do_flat_print(disas, ai);
			j++;
			break;
		}
		fputc('\n', stdout);
		if(ret < 0)
			break;
		i += ret + j - 1;
	}
	arch_delete(disas->arch);
	return -ret;
}

static int _do_flat_print(Disas * disas, ArchInstruction * ai)
{
	int ret = 0;
	int i;
	ArchOperands operands;
	unsigned int reg;
	ArchRegister * ar;
	char const * sep = " ";
	uint8_t size;
	int j;
	unsigned long u;
	int c;

	printf("\t\t%s", ai->name);
	for(i = 0, operands = ai->operands; operands > 0; i++, operands >>= 8)
		if((operands & _AO_OP) == _AO_REG)
		{
			reg = (operands & 0xff) >> 2;
			if((ar = arch_register_get_by_id(disas->arch, reg))
					!= NULL)
				printf("%s%%%s", sep, ar->name);
			else
				printf("%s%d", sep, reg);
			sep = ", ";
		}
		else if((operands & _AO_OP) == _AO_IMM)
		{
			size = (i == 0) ? ai->op1size : ((i == 1) ? ai->op2size
					: ai->op3size);
			for(j = 0, u = 0; j < size; j++)
			{
				if((c = fgetc(disas->fp)) == EOF)
				{
					ret = _disas_error(disas->filename, -1);
					break;
				}
				u = (u << 8) | c; /* XXX endian */
			}
			if(j != size)
				break;
			printf("%s$0x%lx", sep, u);
			sep = ", ";
			ret += size;
		}
	return ret;
}


/* disas_elf */
/* ELF32 */
static int _do_elf32(Disas * disas, Elf32_Ehdr * ehdr);
static Elf32_Shdr * _do_elf32_shdr(char const * filename, FILE * fp,
		Elf32_Ehdr * ehdr);
static int _do_elf32_strtab(Disas * disas, Elf32_Shdr * shdr, size_t shdr_cnt,
		uint16_t ndx, char ** strtab, size_t * strtab_cnt);
/* ELF64 */
static int _do_elf64(Disas * disas, Elf64_Ehdr * ehdr);
static Elf64_Shdr * _do_elf64_shdr(char const * filename, FILE * fp,
		Elf64_Ehdr * ehdr);
static int _do_elf64_strtab(Disas * disas, Elf64_Shdr * shdr, size_t shdr_cnt,
		uint16_t ndx, char ** strtab, size_t * strtab_cnt);

static int _disas_elf(Disas * disas)
{
	union {
		unsigned char e_ident[EI_NIDENT];
		Elf32_Ehdr ehdr32;
		Elf64_Ehdr ehdr64;
	} u;

	if(fseek(disas->fp, 0, SEEK_SET) != 0)
		return _disas_error(disas->filename, 1);
	if(fread(&u, sizeof(u), 1, disas->fp) != 1)
	{
		fprintf(stderr, "disas: %s: %s\n", disas->filename,
				"Could not determine ELF class");
		return 1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() endian: 0x%x\n", __func__,
			u.e_ident[EI_DATA]);
#endif
	switch(u.e_ident[EI_CLASS])
	{
		case ELFCLASS32:
			return _do_elf32(disas, &u.ehdr32);
		case ELFCLASS64:
			return _do_elf64(disas, &u.ehdr64);
	}
	fprintf(stderr, "disas: %s: %s 0x%x\n", disas->filename,
			"Unsupported ELF class ", u.e_ident[EI_CLASS]);
	return 1;
}

static int _do_elf32(Disas * disas, Elf32_Ehdr * ehdr)
{
	Elf32_Shdr * shdr;
	char * shstrtab = NULL;
	size_t shstrtab_cnt = 0;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() e_shnum=%u\n", __func__, ehdr->e_shnum);
#endif
	if((shdr = _do_elf32_shdr(disas->filename, disas->fp, ehdr)) == NULL)
		return 1;
	if(_do_elf32_strtab(disas, shdr, ehdr->e_shnum, ehdr->e_shstrndx,
				&shstrtab, &shstrtab_cnt) != 0)
	{
		free(shdr);
		return 1;
	}
	for(i = 0; i < ehdr->e_shnum; i++)
	{
		if(shdr[i].sh_name >= shstrtab_cnt)
			continue;
		if(strcmp(".text", &shstrtab[shdr[i].sh_name]) == 0)
		{
			printf("\nDisassembly of section %s:\n",
					&shstrtab[shdr[i].sh_name]);
			_do_flat(disas, shdr[i].sh_offset, shdr[i].sh_size);
		}
	}
	free(shstrtab);
	free(shdr);
	return 0;
}

static Elf32_Shdr * _do_elf32_shdr(char const * filename, FILE * fp,
		Elf32_Ehdr * ehdr)
{
	Elf32_Shdr * shdr;

	if(ehdr->e_shentsize != sizeof(*shdr))
	{
		fprintf(stderr, "disas: %s: %s\n", filename,
				"Invalid section header size");
		return NULL;
	}
	if(fseek(fp, ehdr->e_shoff, SEEK_SET) != 0
			|| (shdr = malloc(ehdr->e_shentsize * ehdr->e_shnum))
			== NULL)
	{
		_disas_error(filename, 1);
		return NULL;
	}
	if(fread(shdr, sizeof(*shdr), ehdr->e_shnum, fp) != ehdr->e_shnum)
	{
		fprintf(stderr, "disas: %s: %s\n", filename, "Short read");
		return NULL;
	}
	return shdr;
}

static int _do_elf32_strtab(Disas * disas, Elf32_Shdr * shdr, size_t shdr_cnt,
		uint16_t ndx, char ** strtab, size_t * strtab_cnt)
{
	if(ndx >= shdr_cnt)
		return 1;
	shdr = &shdr[ndx];
	if(fseek(disas->fp, shdr->sh_offset, SEEK_SET) != 0
			|| (*strtab = malloc(shdr->sh_size)) == NULL)
		return _disas_error(disas->filename, 1);
	if(fread(*strtab, sizeof(**strtab), shdr->sh_size, disas->fp)
			!= shdr->sh_size)
	{
		free(*strtab);
		fprintf(stderr, "disas: %s: %s\n", disas->filename,
				"Short read");
		return 1;
	}
	*strtab_cnt = shdr->sh_size;
	return 0;
}

static int _do_elf64(Disas * disas, Elf64_Ehdr * ehdr)
{
	Elf64_Shdr * shdr;
	char * shstrtab = NULL;
	size_t shstrtab_cnt = 0;
	size_t i;

	if((shdr = _do_elf64_shdr(disas->filename, disas->fp, ehdr)) == NULL)
		return 1;
	if(_do_elf64_strtab(disas, shdr, ehdr->e_shnum, ehdr->e_shstrndx,
				&shstrtab, &shstrtab_cnt) != 0)
	{
		free(shdr);
		return 1;
	}
	for(i = 0; i < ehdr->e_shnum; i++)
	{
		if(shdr[i].sh_name >= shstrtab_cnt)
			continue;
		if(strcmp(".text", &shstrtab[shdr[i].sh_name]) == 0)
			_do_flat(disas, shdr[i].sh_offset, shdr[i].sh_size);
	}
	free(shstrtab);
	free(shdr);
	return 0;
}

static Elf64_Shdr * _do_elf64_shdr(char const * filename, FILE * fp,
		Elf64_Ehdr * ehdr)
{
	Elf64_Shdr * shdr;

	if(ehdr->e_shentsize != sizeof(*shdr))
	{
		fprintf(stderr, "disas: %s: %s\n", filename,
				"Invalid section header size");
		return NULL;
	}
	if(fseek(fp, ehdr->e_shoff, SEEK_SET) != 0
			|| (shdr = malloc(ehdr->e_shentsize * ehdr->e_shnum))
			== NULL)
	{
		_disas_error(filename, 1);
		return NULL;
	}
	if(fread(shdr, sizeof(*shdr), ehdr->e_shnum, fp) != ehdr->e_shnum)
	{
		fprintf(stderr, "disas: %s: %s\n", filename, "Short read");
		return NULL;
	}
	return shdr;
}

static int _do_elf64_strtab(Disas * disas, Elf64_Shdr * shdr, size_t shdr_cnt,
		uint16_t ndx, char ** strtab, size_t * strtab_cnt)
{
	if(ndx >= shdr_cnt)
		return 1;
	shdr = &shdr[ndx];
	if(fseek(disas->fp, shdr->sh_offset, SEEK_SET) != 0
			|| (*strtab = malloc(shdr->sh_size)) == NULL)
		return _disas_error(disas->filename, 1);
	if(fread(*strtab, sizeof(**strtab), shdr->sh_size, disas->fp)
			!= shdr->sh_size)
	{
		free(*strtab);
		fprintf(stderr, "disas: %s: %s\n", disas->filename,
				"Short read");
		return 1;
	}
	*strtab_cnt = shdr->sh_size;
	return 0;
}


/* disas_flat */
static int _disas_flat(Disas * disas)
{
	struct stat st;

	if(fstat(fileno(disas->fp), &st) != 0)
		return _disas_error(disas->filename, 1);
	return _do_flat(disas, 0, st.st_size);
}


/* disas_list */
static int _disas_list(void)
{
	size_t i;

	as_plugin_list(ASPT_ARCH);
	fprintf(stderr, "%s", "Available format plug-ins:");
	for(i = 0; i < _disas_signatures_cnt; i++)
		fprintf(stderr, " %s", _disas_signatures[i].name);
	fputc('\n', stderr);
	return 0;
}


/* disas_error */
static int _disas_error(char const * message, int ret)
{
	fputs("disas: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: disas [-a arch][-f format] filename\n"
"       disas -l\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * arch = NULL;
	char const * format = NULL;

	while((o = getopt(argc, argv, "a:f:l")) != -1)
		switch(o)
		{
			case 'a':
				arch = optarg;
				break;
			case 'f':
				format = optarg;
				break;
			case 'l':
				return _disas_list();
			default:
				return _usage();
		}
	if(optind + 1 != argc)
		return _usage();
	return (_disas(arch, format, argv[optind])) == 0 ? 0 : 2;
}
