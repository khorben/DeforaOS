/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Loader */
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



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <elf.h>


/* private */
/* prototypes */
static int _ldd(char const * filename);

static int _error(char const * error1, char const * error2, int ret);
static int _usage(void);


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int ret = 0;
	int o;
	int i;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	for(i = optind; i < argc; i++)
		ret |= _ldd(argv[i]);
	return (ret == 0) ? 0 : 2;
}


/* private */
/* functions */
/* ldd */
static int _ldd_do32(char const * filename, FILE * fp, Elf32_Ehdr * ehdr);
static int _ldd_do64(char const * filename, FILE * fp, Elf64_Ehdr * ehdr);
static int _do64_phdr(char const * filename, FILE * fp, Elf64_Ehdr * ehdr,
		Elf64_Phdr * phdr);
static int _do64_phdr_dyn(char const * filename, FILE * fp, Elf64_Ehdr * ehdr,
		Elf64_Phdr * phdr, Elf64_Dyn * dyn);
static char * _do64_string(char const * filename, FILE * fp, Elf64_Ehdr * ehdr,
		Elf64_Addr addr, Elf64_Xword index);

static int _ldd(char const * filename)
{
	int ret = 1;
	FILE * fp;
	union
	{
		unsigned char e_ident[EI_NIDENT];
		Elf32_Ehdr ehdr32;
		Elf64_Ehdr ehdr64;
	} elf;

	if((fp = fopen(filename, "r")) == NULL)
		return _error(filename, strerror(errno), 1);
	if(fread(&elf, sizeof(elf), 1, fp) == 1)
	{
		if(memcmp(elf.e_ident, ELFMAG, SELFMAG) != 0)
			ret = -_error(filename, "Not an ELF file", 1);
		else if(elf.e_ident[EI_CLASS] == ELFCLASS32)
			ret = _ldd_do32(filename, fp, &elf.ehdr32);
		else if(elf.e_ident[EI_CLASS] == ELFCLASS64)
			ret = _ldd_do64(filename, fp, &elf.ehdr64);
		else
			ret = -_error(filename, "Could not determine ELF class",
					1);
	}
	else if(ferror(fp) != 0)
		ret = -_error(filename, strerror(errno), 1);
	else
		ret = -_error(filename, "Not an ELF file", 1);
	if(fclose(fp) != 0)
		ret = -_error(filename, strerror(errno), 1);
	return ret;
}

static int _ldd_do32(char const * filename, FILE * fp, Elf32_Ehdr * ehdr)
{
	/* FIXME implement */
	return 0;
}

static int _ldd_do64(char const * filename, FILE * fp, Elf64_Ehdr * ehdr)
{
	int ret;
	Elf64_Phdr * phdr;

	if(ehdr->e_phnum == 0)
		return -_error(filename, "No program header found", 1);
	if(ehdr->e_phentsize != sizeof(*phdr))
		return -_error(filename, "Unexpected program header size", 1);
	if(fseek(fp, ehdr->e_phoff, SEEK_SET) != 0)
		return -_error(filename, strerror(errno), 1);
	if((phdr = malloc(sizeof(*phdr) * ehdr->e_phnum)) == NULL)
		return -_error(filename, strerror(errno), 1);
	ret = _do64_phdr(filename, fp, ehdr, phdr);
	free(phdr);
	return ret;
}

static int _do64_phdr(char const * filename, FILE * fp, Elf64_Ehdr * ehdr,
		Elf64_Phdr * phdr)
{
	int ret = 0;
	size_t i;
	Elf64_Dyn * dyn;

	if(fread(phdr, sizeof(*phdr), ehdr->e_phnum, fp) != ehdr->e_phnum)
	{
		if(ferror(fp) != 0)
			ret = -_error(filename, strerror(errno), 1);
		else
			ret = -_error(filename, "Corrupted ELF file", 1);
		return ret;
	}
	printf("%s:\n", filename);
	for(i = 0; i < ehdr->e_phnum; i++)
	{
		if(phdr[i].p_type != PT_DYNAMIC)
			continue;
		if(fseek(fp, phdr[i].p_offset, SEEK_SET) != 0)
		{
			ret |= -_error(filename, strerror(errno), 1);
			continue;
		}
		if((dyn = malloc(phdr[i].p_filesz)) == NULL)
		{
			ret |= -_error(filename, strerror(errno), 1);
			continue;
		}
		if(fread(dyn, phdr[i].p_filesz, 1, fp) == 1)
			ret = _do64_phdr_dyn(filename, fp, ehdr, &phdr[i], dyn);
		else
		{
			if(ferror(fp) != 0)
				ret |= -_error(filename, strerror(errno), 1);
			else
				ret |= -_error(filename, "Corrupted ELF file",
						1);
		}
		free(dyn);
	}
	return ret;
}

static int _do64_phdr_dyn(char const * filename, FILE * fp, Elf64_Ehdr * ehdr,
		Elf64_Phdr * phdr, Elf64_Dyn * dyn)
{
	ssize_t s = -1;
	ssize_t r = -1;
	size_t i;
	char * p;

	for(i = 0; (i + 1) * sizeof(*dyn) < phdr->p_filesz; i++)
	{
		if(dyn[i].d_tag == DT_NULL)
			break;
		if(dyn[i].d_tag == DT_STRTAB)
			s = i;
		else if(dyn[i].d_tag == DT_RPATH)
			r = i;
	}
	if(s < 0)
		return -_error(filename, "Missing string section", 1);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() strtab=%ld\n", __func__, s);
#endif
	for(i = 0; (i + 1) * sizeof(*dyn) < phdr->p_filesz; i++)
	{
		if(dyn[i].d_tag == DT_NULL)
			break;
		if(dyn[i].d_tag != DT_NEEDED)
			continue;
		if((p = _do64_string(filename, fp, ehdr, dyn[s].d_un.d_ptr,
						dyn[i].d_un.d_val)) == NULL)
			continue;
		printf("\t%s\n", p);
		/* FIXME display the full filename and address */
		free(p);
	}
	return 0;
}

static char * _do64_string(char const * filename, FILE * fp, Elf64_Ehdr * ehdr,
		Elf64_Addr addr, Elf64_Xword index)
{
	char * ret;
	size_t i;
	Elf64_Shdr shdr;
	char * p = NULL;

	if(fseek(fp, ehdr->e_shoff, SEEK_SET) != 0)
	{
		_error(filename, strerror(errno), 1);
		return NULL;
	}
	for(i = 0; i < ehdr->e_shnum; i++)
	{
		if(fread(&shdr, sizeof(shdr), 1, fp) != 1)
		{
			if(ferror(fp) != 0)
				_error(filename, strerror(errno), 1);
			else
				_error(filename, "Corrupted ELF file", 1);
			break;
		}
		if(shdr.sh_type != SHT_STRTAB)
			continue;
		if(shdr.sh_addr != addr)
			continue;
		if(fseek(fp, shdr.sh_offset, SEEK_SET) != 0
				|| (p = malloc(shdr.sh_size)) == NULL)
		{
			_error(filename, strerror(errno), 1);
			break;
		}
		if(fread(p, sizeof(*p), shdr.sh_size, fp) != shdr.sh_size)
		{
			if(ferror(fp) != 0)
				_error(filename, strerror(errno), 1);
			else
				_error(filename, "Corrupted ELF file", 1);
			break;
		}
		for(i = index; i < shdr.sh_size; i++)
		{
			if(p[i] != '\0')
				continue;
			if((ret = strdup(&p[index])) == NULL)
				_error(filename, strerror(errno), 1);
			free(p);
			return ret;
		}
		break;
	}
	free(p);
	return NULL;
}


/* error */
static int _error(char const * error1, char const * error2, int ret)
{
	fprintf(stderr, "%s: %s%s%s\n", "ldd", error1, (error2 != NULL) ? ": "
			: "", (error2 != NULL) ? error2 : "");
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: ldd filename...\n", stderr);
	return 1;
}
