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
#ifndef EM_486
# define EM_486 6
#endif
#include "as.h"
#include "arch/arch.h"


/* disas */
/* private */
/* types */
typedef struct _Disas
{
	char const * filename;
	FILE * fp;
	char * archname;
	Arch * arch;

	/* ELF */
	union
	{
		unsigned char e_ident[EI_NIDENT];
		Elf32_Ehdr ehdr32;
		Elf64_Ehdr ehdr64;
	} elf;
} Disas;

typedef struct _DisasSignature
{
	char const * format;
	char const * signature;
	size_t size;
	int (*detect)(Disas * disas);
	int (*disas)(Disas * disas);
} DisasSignature;


/* variables */
static int _elf_detect(Disas * disas);
static int _elf_disas32(Disas * disas);
static int _elf_disas64(Disas * disas);
static int _flat_disas(Disas * disas);
static int _java_detect(Disas * disas);
static int _java_disas(Disas * disas);

static DisasSignature _disas_signatures[] =
{
	{ "elf",	ELFMAG, SELFMAG,	_elf_detect, NULL },
	{ "java",	"\xca\xfe\xba\xbe", 4,	_java_detect, _java_disas },
	{ "flat",	NULL, 0,		NULL, _flat_disas }
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

	if(arch == NULL)
		disas.archname = NULL;
	else if((disas.archname = strdup(arch)) == NULL)
		return -_disas_error(filename, 1);
	if((disas.fp = fopen(filename, "r")) == NULL)
	{
		free(disas.archname);
		return -_disas_error(filename, 1);
	}
	disas.filename = filename;
	disas.arch = NULL;
	ret = _disas_do_format(&disas, format);
	free(disas.archname);
	fclose(disas.fp);
	return ret;
}

static int _disas_do_format(Disas * disas, char const * format)
{
	size_t i;

	if(format == NULL)
		return _disas_do(disas);
	for(i = 0; i < _disas_signatures_cnt; i++)
		if(strcmp(_disas_signatures[i].format, format) == 0)
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
	int ret;
	As * as;

	if(_disas_signatures[i].detect != NULL
			&& _disas_signatures[i].detect(disas) != 0)
		return -1;
	if((as = as_new(disas->archname, _disas_signatures[i].format)) == NULL)
		return -1;
	printf("\n%s: %s-%s\n", disas->filename, _disas_signatures[i].format,
			as_get_arch(as));
	if(disas->arch != NULL)
		arch_delete(disas->arch);
	disas->arch = arch_new(as_get_arch(as));
	as_delete(as);
	if(disas->arch == NULL)
		return -1;
	ret = _disas_signatures[i].disas(disas);
	return ret;
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


/* elf_detect */
static char const * _elf_detect32(Disas * disas);
static char const * _elf_detect64(Disas * disas);

static int _elf_detect(Disas * disas)
{
	char const * archname;
	char * p;

	if(fseek(disas->fp, 0, SEEK_SET) != 0)
		return _disas_error(disas->filename, 1);
	if(fread(&disas->elf, sizeof(disas->elf), 1, disas->fp) != 1)
	{
		fprintf(stderr, "disas: %s: %s\n", disas->filename,
				"Could not determine ELF class");
		return 1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() endian: 0x%x\n", __func__,
			disas->elf.e_ident[EI_DATA]);
#endif
	switch(disas->elf.e_ident[EI_CLASS])
	{
		case ELFCLASS32:
			archname = _elf_detect32(disas);
			break;
		case ELFCLASS64:
			archname = _elf_detect64(disas);
			break;
		default:
			fprintf(stderr, "disas: %s: %s 0x%x\n", disas->filename,
					"Unsupported ELF class",
					disas->elf.e_ident[EI_CLASS]);
			return -1;
	}
	if(archname == NULL)
		return -1;
	if((p = strdup(archname)) == NULL)
		return -1;
	free(disas->archname);
	disas->archname = p;
	return 0;
}

static char const * _elf_detect32(Disas * disas)
{
	_disas_signatures[0].disas = _elf_disas32; /* XXX hard-coded */
	switch(disas->elf.ehdr32.e_machine)
	{
		case EM_SPARC:
			return "sparc";
		case EM_386:
		case EM_486:
			return "i686"; /* XXX i386? i486? */
		case EM_MIPS:
			return "mips";
		case EM_ARM:
			return "arm";
		case EM_ALPHA:
			return "alpha";
	}
	fprintf(stderr, "disas: %s: %s 0x%x\n", disas->filename,
			"Unsupported ELF architecture",
			disas->elf.ehdr32.e_machine);
	return NULL;
}

static char const * _elf_detect64(Disas * disas)
{
	_disas_signatures[0].disas = _elf_disas64; /* XXX hard-coded */
	switch(disas->elf.ehdr64.e_machine)
	{
		case EM_SPARC:
		case EM_SPARCV9:
			return "sparc64";
		case EM_X86_64:
			return "amd64";
	}
	fprintf(stderr, "disas: %s: %s 0x%x\n", disas->filename,
			"Unsupported ELF architecture",
			disas->elf.ehdr64.e_machine);
	return NULL;
}


/* elf_disas32 */
static Elf32_Shdr * _do_elf32_shdr(char const * filename, FILE * fp,
		Elf32_Ehdr * ehdr);
static int _do_elf32_strtab(Disas * disas, Elf32_Shdr * shdr, size_t shdr_cnt,
		uint16_t ndx, char ** strtab, size_t * strtab_cnt);

static int _elf_disas32(Disas * disas)
{
	Elf32_Ehdr * ehdr = &disas->elf.ehdr32;
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
		if(strcmp(".text", &shstrtab[shdr[i].sh_name]) == 0
				|| strcmp(".init", &shstrtab[shdr[i].sh_name])
				== 0)
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


/* elf_disas64 */
static Elf64_Shdr * _do_elf64_shdr(char const * filename, FILE * fp,
		Elf64_Ehdr * ehdr);
static int _do_elf64_strtab(Disas * disas, Elf64_Shdr * shdr, size_t shdr_cnt,
		uint16_t ndx, char ** strtab, size_t * strtab_cnt);

static int _elf_disas64(Disas * disas)
{
	Elf64_Ehdr * ehdr = &disas->elf.ehdr64;
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
		if(strcmp(".text", &shstrtab[shdr[i].sh_name]) == 0
				|| strcmp(".init", &shstrtab[shdr[i].sh_name])
				== 0)
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


/* flat_disas */
static int _flat_disas(Disas * disas)
{
	struct stat st;

	if(fstat(fileno(disas->fp), &st) != 0)
		return _disas_error(disas->filename, 1);
	return _do_flat(disas, 0, st.st_size);
}


/* java_detect */
static int _java_detect(Disas * disas)
{
	char * p;

	if((p = strdup("java")) == NULL)
		return -1;
	free(disas->archname);
	disas->archname = p;
	return 0;
}


/* java_disas */
static int _java_disas(Disas * disas)
{
	struct stat st;

	if(fstat(fileno(disas->fp), &st) != 0)
		return _disas_error(disas->filename, 1);
	return _do_flat(disas, 8, st.st_size - 8);
}


/* disas_list */
static int _disas_list(void)
{
	size_t i;

	as_plugin_list(ASPT_ARCH);
	fprintf(stderr, "%s", "Available format plug-ins:");
	for(i = 0; i < _disas_signatures_cnt; i++)
		fprintf(stderr, " %s", _disas_signatures[i].format);
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
