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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#ifndef EM_486
# define EM_486 6
#endif
#include "As/as.h"
#include "arch.h"
#include "as.h"

#define min(a, b) ((a) < (b) ? (a) : (b))


/* disas */
/* private */
/* types */
typedef struct _Disas
{
	char const * filename;
	FILE * fp;
	char * arch;
	As * as;

	/* ELF */
	union
	{
		unsigned char e_ident[EI_NIDENT];
		Elf32_Ehdr ehdr32;
		Elf64_Ehdr ehdr64;
	} elf;
} Disas;

/* FIXME use signatures from the format plug-ins directly */
typedef struct _DisasSignature
{
	char const * format;
	char const * signature;
	size_t size;
	int (*detect)(Disas * disas);
	int (*callback)(Disas * disas);
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
	{ "elf",	ELFMAG, SELFMAG,	_elf_detect,	NULL },
	{ "flat",	NULL, 0,		NULL,		_flat_disas },
	{ "java",	"\xca\xfe\xba\xbe", 4,	_java_detect,	_java_disas }
};
#define _disas_signatures_cnt (sizeof(_disas_signatures) \
		/ sizeof(*_disas_signatures))


/* prototypes */
static int _disas(char const * arch, char const * format,
		char const * filename);
static int _disas_buffer(char const * arch, char const * format,
		char const * buffer, size_t size);
static int _disas_string(char const * arch, char const * format,
		char const * string);
static int _disas_list(void);

static int _disas_error(char const * message, int ret);

static int _ishex(int c);
static int _hex2bin(int c);


/* functions */
/* disas */
static int _disas_do_format(Disas * disas, char const * format);
static int _disas_do(Disas * disas);
static int _do_callback(Disas * disas, size_t i);
static int _do_flat(Disas * disas, off_t offset, size_t size, off_t base);
static void _do_flat_print(Arch * arch, unsigned long address,
		char const * buffer, size_t size, ArchInstruction * ai);

static int _disas(char const * arch, char const * format, char const * filename)
{
	int ret = 1;
	Disas disas;

	disas.as = NULL;
	if(arch == NULL)
		disas.arch = NULL;
	else if((disas.arch = strdup(arch)) == NULL)
		return -_disas_error(filename, 1);
	if((disas.fp = fopen(filename, "r")) == NULL)
	{
		free(disas.arch);
		return -_disas_error(filename, 1);
	}
	disas.filename = filename;
	ret = _disas_do_format(&disas, format);
	free(disas.arch);
	if(disas.as != NULL)
		as_delete(disas.as);
	fclose(disas.fp);
	return ret;
}

static int _disas_do_format(Disas * disas, char const * format)
{
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, format);
#endif
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

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	for(i = 0; i < _disas_signatures_cnt; i++)
		if(_disas_signatures[i].size > s)
			s = _disas_signatures[i].size;
	if((buf = malloc(s)) == NULL)
	{
		free(buf);
		return -_disas_error(disas->filename, 1);
	}
	if(fread(buf, sizeof(*buf), s, disas->fp) != s)
	{
		if(ferror(disas->fp))
			ret = -_disas_error(disas->filename, 1);
		else
			ret = _disas_do_format(disas, "flat");
	}
	else
	{
		for(i = 0; i < _disas_signatures_cnt; i++)
			if(_disas_signatures[i].size == 0)
				continue;
			else if(memcmp(_disas_signatures[i].signature, buf,
						_disas_signatures[i].size) == 0)
			{
				ret = _do_callback(disas, i);
				break;
			}
		if(i == _disas_signatures_cnt)
			ret = _disas_do_format(disas, "flat");
	}
	free(buf);
	return ret;
}

static int _do_callback(Disas * disas, size_t i)
{
	int ret;

	if(_disas_signatures[i].detect != NULL
			&& _disas_signatures[i].detect(disas) != 0)
		return -1;
	if((disas->as = as_new(disas->arch, _disas_signatures[i].format))
			== NULL)
		return -1;
	printf("\n%s: %s-%s\n", disas->filename, _disas_signatures[i].format,
			as_get_arch_name(disas->as));
	ret = _disas_signatures[i].callback(disas);
	return ret;
}

static int _do_flat(Disas * disas, off_t offset, size_t size, off_t base)
{
	int ret = 0;
	Arch * arch = as_get_arch(disas->as);
	size_t pos;
	char buf[8];
	size_t buf_cnt = 0;
	size_t cnt;
	ArchInstruction * ai;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, 0x%lx, 0x%lx) arch=\"%s\"\n", __func__,
			(void *)disas, (unsigned long)offset, size,
			as_get_arch_name(disas->as));
#endif
	if(fseek(disas->fp, offset, SEEK_SET) != 0)
		return -_disas_error(disas->filename, 1);
	printf("\n%08lx:\n", (unsigned long)offset + base);
	memset(buf, 0, sizeof(buf));
	for(pos = 0; pos < size; pos += cnt)
	{
		if((cnt = sizeof(buf) - buf_cnt) > 0 && size - pos - 1 > 0)
		{
			if((cnt = fread(&buf[buf_cnt], 1, cnt, disas->fp)) == 0)
				break;
			buf_cnt += cnt;
		}
		cnt = buf_cnt;
		if((ai = as_decode(disas->as, buf, &cnt)) != NULL)
			_do_flat_print(arch, base + offset + pos, buf, cnt, ai);
		else
			cnt = 1; /* FIXME print missing instruction */
		memmove(buf, &buf[cnt], buf_cnt - cnt);
		buf_cnt -= cnt;
	}
	if(ferror(disas->fp))
		return -_disas_error(disas->filename, 1);
	return ret;
}

static void _do_flat_print(Arch * arch, unsigned long address,
		char const * buffer, size_t size, ArchInstruction * ai)
{
	size_t pos = ai->size;
	size_t i;
	int col;
	ArchOperands operands;
	unsigned int reg;
	ArchRegister * ar;
	char const * sep = " ";
	unsigned long u;
	size_t j;
	size_t s;

	col = printf(" %5lx:", address);
	for(i = 0; i < size; i++)
		col += printf(" %02x", (unsigned char)buffer[i]);
	for(; col < 31; col++)
		putchar(' ');
	printf(" %s", ai->name);
	for(i = 0, operands = ai->operands; operands > 0; i++, operands >>= 8)
		if((operands & _AO_OP) == _AO_REG)
		{
			reg = (operands & 0xff) >> 2;
			if((ar = arch_register_get_by_id(arch, reg))
					!= NULL)
				printf("%s%%%s", sep, ar->name);
			else
				printf("%s%d", sep, reg);
			sep = ", ";
		}
		else if((operands & _AO_OP) == _AO_DREG)
		{
			reg = (operands & 0xff) >> 2;
			if((ar = arch_register_get_by_id(arch, reg))
					!= NULL)
				printf("%s(%%%s)", sep, ar->name);
			else
				printf("%s(%d)", sep, reg);
			sep = ", ";
		}
		else if((operands & _AO_OP) == _AO_IMM)
		{
			s = (i == 0) ? ai->op1size : ((i == 1) ? ai->op2size
					: ai->op3size);
			for(j = 0, u = 0; j < s; j++)
				u = (u << 8) | (unsigned char)buffer[pos++];
			/* XXX fix endian */
			printf("%s$0x%lx", sep, u);
			sep = ", ";
		}
	putchar('\n');
}


/* disas_buffer */
static int _disas_buffer(char const * arch, char const * format,
		char const * buffer, size_t size)
{
	As * as;
	Arch * a;
	size_t pos;
	size_t cnt;
	ArchInstruction * ai;

	if((as = as_new(arch, format)) == NULL)
		return -1;
	if((a = as_get_arch(as)) == NULL)
	{
		as_delete(as);
		return -1;
	}
	for(pos = 0; pos < size; pos += cnt)
	{
		cnt = size - pos;
		if((ai = as_decode(as, &buffer[pos], &cnt)) != NULL)
			_do_flat_print(a, pos, &buffer[pos], cnt, ai);
		else
			cnt = 1;
	}
	as_delete(as);
	return 0;
}


/* disas_string */
static int _disas_string(char const * arch, char const * format,
		char const * string)
{
	int ret;
	unsigned char * str = (unsigned char *)string;
	size_t len = strlen(string);
	char * s;
	size_t i;
	size_t j;

	if((s = malloc(len + 1)) == NULL)
		return -_disas_error("string", 1);
	for(i = 0, j = 0; i < len; i++)
	{
		if(str[i] != '\\')
			s[j++] = str[i];
		else if(str[i + 1] != 'x') /* "\\" */
			s[j++] = str[++i];
		else if(i + 3 < len && _ishex(str[i + 2])
				&& _ishex(str[i + 3])) /* "\xHH" */
		{
			s[j++] = (_hex2bin(str[i + 2]) << 4)
				| _hex2bin(str[i + 3]);
			i += 3;
		}
	}
	s[j] = '\0'; /* not really necessary */
	ret = _disas_buffer(arch, format, s, j);
	free(s);
	return ret;
}


/* elf_detect */
static char const * _elf_detect32(Disas * disas);
static char const * _elf_detect64(Disas * disas);

static int _elf_detect(Disas * disas)
{
	char const * arch;
	char * p;

	if(fseek(disas->fp, 0, SEEK_SET) != 0)
		return -_disas_error(disas->filename, 1);
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
			arch = _elf_detect32(disas);
			break;
		case ELFCLASS64:
			arch = _elf_detect64(disas);
			break;
		default:
			fprintf(stderr, "disas: %s: %s 0x%x\n", disas->filename,
					"Unsupported ELF class",
					disas->elf.e_ident[EI_CLASS]);
			return -1;
	}
	if(arch == NULL)
		return -1;
	if((p = strdup(arch)) == NULL)
		return -1;
	free(disas->arch);
	disas->arch = p;
	return 0;
}

static char const * _elf_detect32(Disas * disas)
{
	_disas_signatures[0].callback = _elf_disas32; /* XXX hard-coded */
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
	_disas_signatures[0].callback = _elf_disas64; /* XXX hard-coded */
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
static int _do_elf32_addr(char const * filename, FILE * fp, Elf32_Ehdr * ehdr,
		Elf32_Addr * addr);
static int _do_elf32_strtab(Disas * disas, Elf32_Shdr * shdr, size_t shdr_cnt,
		uint16_t ndx, char ** strtab, size_t * strtab_cnt);

static int _elf_disas32(Disas * disas)
{
	Elf32_Ehdr * ehdr = &disas->elf.ehdr32;
	Elf32_Shdr * shdr;
	Elf32_Addr base = 0x0;
	char * shstrtab = NULL;
	size_t shstrtab_cnt = 0;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() e_shnum=%u\n", __func__, ehdr->e_shnum);
#endif
	if((shdr = _do_elf32_shdr(disas->filename, disas->fp, ehdr)) == NULL)
		return 1;
	if(_do_elf32_addr(disas->filename, disas->fp, ehdr, &base) != 0
			|| _do_elf32_strtab(disas, shdr, ehdr->e_shnum,
				ehdr->e_shstrndx, &shstrtab, &shstrtab_cnt)
			!= 0)
	{
		free(shdr);
		return 1;
	}
	for(i = 0; i < ehdr->e_shnum; i++)
	{
		if(shdr[i].sh_name >= shstrtab_cnt)
			continue;
		if(shdr[i].sh_type == SHT_PROGBITS
				&& shdr[i].sh_flags & SHF_EXECINSTR)
		{
			printf("\nDisassembly of section %s:\n",
					&shstrtab[shdr[i].sh_name]);
			_do_flat(disas, shdr[i].sh_offset, shdr[i].sh_size,
					base);
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

static int _do_elf32_addr(char const * filename, FILE * fp, Elf32_Ehdr * ehdr,
		Elf32_Addr * addr)
{
	Elf32_Half i;
	Elf32_Phdr phdr;

	if(fseek(fp, ehdr->e_phoff, SEEK_SET) != 0)
		return -_disas_error(filename, 1);
	for(i = 0; i < ehdr->e_phnum; i++)
		if(fread(&phdr, sizeof(phdr), 1, fp) != 1)
			return -_disas_error(filename, 1);
		else if(phdr.p_type == PT_LOAD && phdr.p_flags & (PF_R | PF_X))
		{
			*addr = phdr.p_vaddr;
			return 0;
		}
	*addr = 0x0;
	return 0;
}

static int _do_elf32_strtab(Disas * disas, Elf32_Shdr * shdr, size_t shdr_cnt,
		uint16_t ndx, char ** strtab, size_t * strtab_cnt)
{
	if(ndx >= shdr_cnt)
		return 1;
	shdr = &shdr[ndx];
	if(fseek(disas->fp, shdr->sh_offset, SEEK_SET) != 0
			|| (*strtab = malloc(shdr->sh_size)) == NULL)
		return -_disas_error(disas->filename, 1);
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
static int _do_elf64_addr(char const * filename, FILE * fp, Elf64_Ehdr * ehdr,
		Elf64_Addr * addr);
static int _do_elf64_strtab(Disas * disas, Elf64_Shdr * shdr, size_t shdr_cnt,
		uint16_t ndx, char ** strtab, size_t * strtab_cnt);

static int _elf_disas64(Disas * disas)
{
	Elf64_Ehdr * ehdr = &disas->elf.ehdr64;
	Elf64_Shdr * shdr;
	Elf64_Addr base = 0x0;
	char * shstrtab = NULL;
	size_t shstrtab_cnt = 0;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() e_shnum=%u\n", __func__, ehdr->e_shnum);
#endif
	if((shdr = _do_elf64_shdr(disas->filename, disas->fp, ehdr)) == NULL)
		return 1;
	if(_do_elf64_addr(disas->filename, disas->fp, ehdr, &base) != 0
			|| _do_elf64_strtab(disas, shdr, ehdr->e_shnum,
				ehdr->e_shstrndx, &shstrtab, &shstrtab_cnt)
			!= 0)
	{
		free(shdr);
		return 1;
	}
	for(i = 0; i < ehdr->e_shnum; i++)
	{
		if(shdr[i].sh_name >= shstrtab_cnt)
			continue;
		if(shdr[i].sh_type == SHT_PROGBITS
				&& shdr[i].sh_flags & SHF_EXECINSTR)
		{
			printf("\nDisassembly of section %s:\n",
					&shstrtab[shdr[i].sh_name]);
			_do_flat(disas, shdr[i].sh_offset, shdr[i].sh_size,
					base);
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

static int _do_elf64_addr(char const * filename, FILE * fp, Elf64_Ehdr * ehdr,
		Elf64_Addr * addr)
{
	Elf64_Quarter i;
	Elf64_Phdr phdr;

	if(fseek(fp, ehdr->e_phoff, SEEK_SET) != 0)
		return -_disas_error(filename, 1);
	for(i = 0; i < ehdr->e_phnum; i++)
		if(fread(&phdr, sizeof(phdr), 1, fp) != 1)
			return -_disas_error(filename, 1);
		else if(phdr.p_type == PT_LOAD && phdr.p_flags & (PF_R | PF_X))
		{
			*addr = phdr.p_vaddr;
			return 0;
		}
	*addr = 0x0;
	return 0;
}

static int _do_elf64_strtab(Disas * disas, Elf64_Shdr * shdr, size_t shdr_cnt,
		uint16_t ndx, char ** strtab, size_t * strtab_cnt)
{
	if(ndx >= shdr_cnt)
		return 1;
	shdr = &shdr[ndx];
	if(fseek(disas->fp, shdr->sh_offset, SEEK_SET) != 0
			|| (*strtab = malloc(shdr->sh_size)) == NULL)
		return -_disas_error(disas->filename, 1);
	if(fread(*strtab, sizeof(**strtab), shdr->sh_size, disas->fp)
			!= shdr->sh_size)
	{
		free(*strtab);
		fprintf(stderr, "disas: %s: %s\n", disas->filename,
				"Short read");
		return -1;
	}
	*strtab_cnt = shdr->sh_size;
	return 0;
}


/* flat_disas */
static int _flat_disas(Disas * disas)
{
	struct stat st;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, disas->filename);
#endif
	if(fstat(fileno(disas->fp), &st) != 0)
		return -_disas_error(disas->filename, 1);
	return _do_flat(disas, 0, st.st_size, 0);
}


/* java_detect */
static int _java_detect(Disas * disas)
{
	char * p;

	if((p = strdup("java")) == NULL)
		return -1;
	free(disas->arch);
	disas->arch = p;
	return 0;
}


/* java_disas */
static int _java_disas(Disas * disas)
{
	struct stat st;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, disas->filename);
#endif
	if(fstat(fileno(disas->fp), &st) != 0)
		return -_disas_error(disas->filename, 1);
	return _do_flat(disas, 8, st.st_size - 8, 0);
}


/* disas_list */
static int _disas_list(void)
{
	size_t i;
	char const * sep = "";

	as_plugin_list(ASPT_ARCH);
	fputs("\nAvailable format plug-ins:\n", stderr);
	for(i = 0; i < _disas_signatures_cnt; i++)
	{
		fprintf(stderr, "%s%s", sep, _disas_signatures[i].format);
		sep = ", ";
	}
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


/* hex2bin */
static int _hex2bin(int c)
{
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}


/* ishex */
static int _ishex(int c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
		|| (c >= 'A' || c <= 'F');
}


/* usage */
static int _usage(void)
{
	fputs("Usage: disas [-a arch][-f format] filename\n"
"       disas [-a arch][-f format] -s string\n"
"       disas -l\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * arch = NULL;
	char const * format = NULL;
	char const * string = NULL;

	while((o = getopt(argc, argv, "a:f:ls:")) != -1)
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
			case 's':
				string = optarg;
				break;
			default:
				return _usage();
		}
	if(optind == argc && string != NULL)
		return _disas_string(arch, format, string);
	else if(optind + 1 == argc && string == NULL)
		return (_disas(arch, format, argv[optind])) == 0 ? 0 : 2;
	else
		return _usage();
}
