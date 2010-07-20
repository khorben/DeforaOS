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



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>


/* disas */
/* private */
/* prototypes */
static int _disas_error(char const * message, int ret);


/* functions */
/* disas */
/* ELF */
static int _do_elf(char const * filename, FILE * fp);
/* ELF32 */
static int _do_elf32(char const * filename, FILE * fp, Elf32_Ehdr * ehdr);
static Elf32_Shdr * _do_elf32_shdr(char const * filename, FILE * fp,
		Elf32_Ehdr * ehdr);
static int _do_elf32_strtab(char const * filename, FILE * fp, Elf32_Shdr * shdr,
		size_t shdr_cnt, uint16_t ndx, char ** strtab,
		size_t * strtab_cnt);
/* ELF64 */
static int _do_elf64(char const * filename, FILE * fp, Elf64_Ehdr * ehdr);
static Elf64_Shdr * _do_elf64_shdr(char const * filename, FILE * fp,
		Elf64_Ehdr * ehdr);
static int _do_elf64_strtab(char const * filename, FILE * fp, Elf64_Shdr * shdr,
		size_t shdr_cnt, uint16_t ndx, char ** strtab,
		size_t * strtab_cnt);

/* Flat */
static int _do_flat(char const * filename, FILE * fp, size_t offset,
		size_t size);

static int _disas(char const * filename)
{
	int ret = 1;
	FILE * fp;
	char buf[32];
	size_t s;
	struct stat st;

	if((fp = fopen(filename, "r")) == NULL)
		return _disas_error(filename, 1);
	s = fread(buf, sizeof(*buf), sizeof(buf), fp);
	if(s > SELFMAG && memcmp(ELFMAG, buf, SELFMAG) == 0)
		ret = _do_elf(filename, fp);
	else if(fstat(fileno(fp), &st) != 0)
		ret = _disas_error(filename, 1);
	else
		ret = _do_flat(filename, fp, 0, st.st_size);
	fclose(fp);
	return ret;
}

static int _do_elf(char const * filename, FILE * fp)
{
	union {
		unsigned char e_ident[EI_NIDENT];
		Elf32_Ehdr ehdr32;
		Elf64_Ehdr ehdr64;
	} u;

	if(fseek(fp, 0, SEEK_SET) != 0)
		return _disas_error(filename, 1);
	if(fread(&u, sizeof(u), 1, fp) != 1)
	{
		fprintf(stderr, "disas: %s: %s\n", filename,
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
			return _do_elf32(filename, fp, &u.ehdr32);
		case ELFCLASS64:
			return _do_elf64(filename, fp, &u.ehdr64);
	}
	fprintf(stderr, "disas: %s: %s 0x%x\n", filename,
			"Unsupported ELF class ", u.e_ident[EI_CLASS]);
	return 1;
}

static int _do_elf32(char const * filename, FILE * fp, Elf32_Ehdr * ehdr)
{
	Elf32_Shdr * shdr;
	char * shstrtab = NULL;
	size_t shstrtab_cnt = 0;
	size_t i;

	if((shdr = _do_elf32_shdr(filename, fp, ehdr)) == NULL)
		return 1;
	if(_do_elf32_strtab(filename, fp, shdr, ehdr->e_shnum, ehdr->e_shstrndx,
				&shstrtab, &shstrtab_cnt) != 0)
	{
		free(shdr);
		return 1;
	}
	printf("\n%s: %s\n", filename, __func__);
	for(i = 0; i < ehdr->e_shnum; i++)
	{
		if(shdr[i].sh_name >= shstrtab_cnt)
			continue;
		if(strcmp(".text", &shstrtab[shdr[i].sh_name]) == 0)
		{
			printf("\nDisassembly of section %s:\n",
					&shstrtab[shdr[i].sh_name]);
			_do_flat(filename, fp, shdr[i].sh_offset,
					shdr[i].sh_size);
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

static int _do_elf32_strtab(char const * filename, FILE * fp, Elf32_Shdr * shdr,
		size_t shdr_cnt, uint16_t ndx, char ** strtab,
		size_t * strtab_cnt)
{
	if(ndx >= shdr_cnt)
		return 1;
	shdr = &shdr[ndx];
	if(fseek(fp, shdr->sh_offset, SEEK_SET) != 0
			|| (*strtab = malloc(shdr->sh_size)) == NULL)
		return _disas_error(filename, 1);
	if(fread(*strtab, sizeof(**strtab), shdr->sh_size, fp) != shdr->sh_size)
	{
		free(*strtab);
		fprintf(stderr, "disas: %s: %s\n", filename, "Short read");
		return 1;
	}
	*strtab_cnt = shdr->sh_size;
	return 0;
}

static int _do_elf64(char const * filename, FILE * fp, Elf64_Ehdr * ehdr)
{
	Elf64_Shdr * shdr;
	char * shstrtab = NULL;
	size_t shstrtab_cnt = 0;
	size_t i;

	if((shdr = _do_elf64_shdr(filename, fp, ehdr)) == NULL)
		return 1;
	if(_do_elf64_strtab(filename, fp, shdr, ehdr->e_shnum, ehdr->e_shstrndx,
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
			_do_flat(filename, fp, shdr[i].sh_offset,
					shdr[i].sh_size);
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

static int _do_elf64_strtab(char const * filename, FILE * fp, Elf64_Shdr * shdr,
		size_t shdr_cnt, uint16_t ndx, char ** strtab,
		size_t * strtab_cnt)
{
	if(ndx >= shdr_cnt)
		return 1;
	shdr = &shdr[ndx];
	if(fseek(fp, shdr->sh_offset, SEEK_SET) != 0
			|| (*strtab = malloc(shdr->sh_size)) == NULL)
		return _disas_error(filename, 1);
	if(fread(*strtab, sizeof(**strtab), shdr->sh_size, fp) != shdr->sh_size)
	{
		free(*strtab);
		fprintf(stderr, "disas: %s: %s\n", filename, "Short read");
		return 1;
	}
	*strtab_cnt = shdr->sh_size;
	return 0;
}

static int _do_flat(char const * filename, FILE * fp, size_t offset,
		size_t size)
{
	size_t i;
	int c;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %p, 0x%zx, 0x%zx\n", __func__,
			filename, fp, offset, size);
#endif
	if(fseek(fp, offset, SEEK_SET) != 0)
		return _disas_error(filename, 1);
	printf("\n%08zx:\n", offset);
	for(i = 0; i < size; i++)
	{
		if((c = fgetc(fp)) == EOF)
			break;
		printf("%5zx:  %02x\n", i, c);
	}
	/* FIXME implement */
	return 1;
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
	fputs("Usage: disas filename\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind + 1 != argc)
		return _usage();
	return (_disas(argv[optind])) == 0 ? 0 : 2;
}
