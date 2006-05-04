/* format/elf.c */
/* FIXME:
 * - understand the section header table
 * - implement string table */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include "format.h"


/* variables */
typedef struct _ElfArch
{
	char * arch;
	unsigned char machine;
	unsigned char capacity;
	unsigned char endian;
} ElfArch;

static ElfArch elf_arch[] =
{
	{ "i386",	EM_386,		ELFCLASS32,	ELFDATA2LSB	},
	{ "i486",	EM_386,		ELFCLASS32,	ELFDATA2LSB	},
	{ "i586",	EM_386,		ELFCLASS32,	ELFDATA2LSB	},
	{ "i686",	EM_386,		ELFCLASS32,	ELFDATA2LSB	},
	{ "sparc",	EM_SPARC,	ELFCLASS32,	ELFDATA2MSB	},
	{ "sparc64",	EM_SPARCV9,	ELFCLASS64,	ELFDATA2MSB	},
	{ NULL,		'\0',		'\0',		'\0'		}
};
static ElfArch * ea;


typedef struct _ElfSectionValues
{
	char * section;
	Elf32_Word type;	/* works for 64-bit too */
	Elf32_Word flags;
} ElfSectionValues;

static ElfSectionValues elf_section_values[] =
{
	{ "bss",	SHT_NOBITS,	SHF_ALLOC | SHF_WRITE		},
	{ "comment",	SHT_PROGBITS,	0				},
	{ "data",	SHT_PROGBITS,	SHF_ALLOC | SHF_WRITE		},
	{ "data1",	SHT_PROGBITS,	SHF_ALLOC | SHF_WRITE		},
	{ "debug",	SHT_PROGBITS,	0				},
	{ "dynamic",	SHT_DYNAMIC,	0				},
	{ "dynstr",	SHT_STRTAB,	SHF_ALLOC			},
	{ "dynsym",	SHT_DYNSYM,	SHF_ALLOC			},
	{ "fini",	SHT_PROGBITS,	SHF_ALLOC | SHF_EXECINSTR	},
	{ "got",	SHT_PROGBITS,	0				},
	{ "hash",	SHT_HASH,	SHF_ALLOC			},
	{ "init",	SHT_PROGBITS,	SHF_ALLOC | SHF_EXECINSTR	},
	{ "interp",	SHT_PROGBITS,	0				},
	{ "line",	SHT_PROGBITS,	0				},
	{ "note",	SHT_NOTE,	0				},
	{ "plt",	SHT_PROGBITS,	0				},
	{ "rodata",	SHT_PROGBITS,	SHF_ALLOC			},
	{ "rodata1",	SHT_PROGBITS,	SHF_ALLOC			},
	{ "shstrtab",	SHT_STRTAB,	0				},
	{ "strtab",	SHT_STRTAB,	0				},
	{ "symtab",	SHT_SYMTAB,	0				},
	{ "text",	SHT_PROGBITS,	SHF_ALLOC | SHF_EXECINSTR	},
	{ NULL,		0,		0				}
};


/* format_plugin */
static int _elf_init(FILE * fp, char * arch);
FormatPlugin format_plugin =
{
	_elf_init,
	NULL,
	NULL
};


/* elf_error */
static int _elf_error(char * message, int ret)
{
	fprintf(stderr, "%s", "as ELF: ");
	perror(message);
	return ret;
}


/* elf_init */
static ElfArch * _init_arch(char * arch);
static int _init_32(FILE * fp);
static int _exit_32(FILE * fp);
static int _section_32(FILE * fp, char * section);
static int _init_64(FILE * fp);
static int _exit_64(FILE * fp);
static int _section_64(FILE * fp, char * section);
static int _elf_init(FILE * fp, char * arch)
{
	if((ea = _init_arch(arch)) == NULL)
		return 1;
	if(ea->capacity == ELFCLASS32)
	{
		/* FIXME use elf_exit to write default sections cross-class if
		 * necessary? */
		format_plugin.exit = _exit_32;
		format_plugin.section = _section_32;
		return _init_32(fp);
	}
	else if(ea->capacity == ELFCLASS64)
	{
		format_plugin.exit = _exit_64;
		format_plugin.section = _section_64;
		return _init_64(fp);
	}
	return 1;
}

static ElfArch * _init_arch(char * arch)
{
	ElfArch * ea;

	for(ea = elf_arch; ea->arch != NULL; ea++)
		if(strcmp(ea->arch, arch) == 0)
			return ea;
	return NULL;
}


/* elf_section */
static ElfSectionValues * _section_values(char * section)
{
	ElfSectionValues * esv;
	int cmp;

	for(esv = elf_section_values; esv->section != NULL; esv++)
		if((cmp = strcmp(esv->section, section)) == 0)
			return esv;
		else if(cmp > 0)
			break;
	for(; esv->section != NULL; esv++);
	return esv;
}


/* elf 32 */
/* variables */
typedef struct _ElfSection32
{
	Elf32_Off offset;
} ElfSection32;
static ElfSection32 * es32 = NULL;
static int es32_cnt = 0;

static int _init_32(FILE * fp)
{
	Elf32_Ehdr hdr;

	memset(&hdr, 0, sizeof(hdr));
	memcpy(&hdr.e_ident, ELFMAG, SELFMAG);
	hdr.e_ident[EI_CLASS] = ELFCLASS32;
	hdr.e_ident[EI_DATA] = ea->endian;
	hdr.e_ident[EI_VERSION] = EV_CURRENT;
	hdr.e_type = ET_REL;
	hdr.e_machine = ea->machine;
	hdr.e_version = EV_CURRENT;
	hdr.e_ehsize = sizeof(hdr);
	hdr.e_shentsize = sizeof(Elf32_Shdr);
	hdr.e_shstrndx = SHN_UNDEF;
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error("fwrite", 1); /* FIXME */
	return 0;
}


static int _exit_32_phdr(FILE * fp);
static int _exit_32_shdr(FILE * fp, long offset);
static int _exit_32_shdr_table(FILE * fp, long offset);
static int _exit_32(FILE * fp)
{
	int ret = 0;
	long offset;

	if((offset = ftell(fp)) == -1)
		ret = _elf_error("ftell", 1); /* FIXME */
	else if(_exit_32_phdr(fp) != 0 || _exit_32_shdr(fp, offset) != 0
			|| _exit_32_shdr_table(fp, offset) != 0)
		ret = 1;
	free(es32);
	return ret;
}

static int _exit_32_phdr(FILE * fp)
{
	Elf32_Ehdr hdr;

	if(es32_cnt == 0)
		return 0;
	if(fseek(fp, 0, SEEK_SET) != 0)
		return _elf_error("fseek", 1);
	if(fread(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error("fread", 1);
	hdr.e_shnum = es32_cnt;
	if(fseek(fp, 0, SEEK_SET) != 0)
		return _elf_error("fseek", 1);
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error("fwrite", 1);
	return 0;
}

static int _exit_32_shdr(FILE * fp, long offset)
{
	Elf32_Shdr hdr;
	int i;

	for(i = 0; i < es32_cnt; i++)
	{
		if(fseek(fp, es32[i].offset, SEEK_SET) != 0)
			return _elf_error("fseek", 1); /* FIXME */
		if(fread(&hdr, sizeof(hdr), 1, fp) != 1)
			return _elf_error("fread", 1); /* FIXME */
		if(i+1 == es32_cnt)
			hdr.sh_size = offset - es32[i].offset - sizeof(hdr);
		else
			hdr.sh_size = es32[i+1].offset - es32[i].offset
				- sizeof(hdr);
		if(fseek(fp, es32[i].offset, SEEK_SET) != 0)
			return _elf_error("fseek", 1); /* FIXME */
		if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
			return _elf_error("fwrite", 1); /* FIXME */
	}
	return 0;
}

static int _exit_32_shdr_table(FILE * fp, long offset)
{
	Elf32_Shdr hdr;

	if(fseek(fp, 0, SEEK_END) != 0)
		return _elf_error("fseek", 1); /* FIXME */
	memset(&hdr, 0, sizeof(hdr));
	hdr.sh_type = SHT_NULL;
	hdr.sh_offset = offset + sizeof(hdr);
	hdr.sh_link = SHN_UNDEF;
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error("fwrite", 1); /* FIXME */
	return 0;
}


static int _section_32(FILE * fp, char * section)
{
	ElfSection32 * p;
	Elf32_Shdr hdr;
	ElfSectionValues * esv;
	long offset;

	if((p = realloc(es32, sizeof(*es32) * (es32_cnt+1))) == NULL)
		return _elf_error("malloc", 1);
	es32 = p;
	p = &es32[es32_cnt++];
	memset(p, 0, sizeof(ElfSection32));
	esv = _section_values(section);
	memset(&hdr, 0, sizeof(hdr));
	hdr.sh_type = esv->type;
	hdr.sh_flags = esv->flags;
	if((offset = ftell(fp)) == -1)
		return _elf_error("ftell", 1); /* FIXME */
	hdr.sh_offset = offset + sizeof(hdr);
	p->offset = offset;
	hdr.sh_link = SHN_UNDEF; /* FIXME */
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error("fwrite", 1); /* FIXME */
	return 0;
}


/* elf 64 */
/* variables */
typedef struct _ElfSection64
{
	Elf64_Off offset;
} ElfSection64;
static ElfSection64 * es64 = NULL;
static int es64_cnt = 0;

static int _init_64(FILE * fp)
{
	Elf64_Ehdr hdr;

	memset(&hdr, 0, sizeof(hdr));
	memcpy(&hdr.e_ident, ELFMAG, SELFMAG);
	hdr.e_ident[EI_CLASS] = ELFCLASS64;
	hdr.e_ident[EI_DATA] = ea->endian;
	hdr.e_ident[EI_VERSION] = EV_CURRENT;
	hdr.e_type = ET_REL;
	hdr.e_machine = ea->machine;
	hdr.e_version = EV_CURRENT;
	hdr.e_ehsize = sizeof(hdr);
	hdr.e_shentsize = sizeof(Elf64_Shdr);
	hdr.e_shstrndx = SHN_UNDEF;
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error("fwrite", 1); /* FIXME */
	return 0;
}


static int _exit_64_phdr(FILE * fp);
static int _exit_64_shdr(FILE * fp, long offset);
static int _exit_64_shdr_table(FILE * fp, long offset);
static int _exit_64(FILE * fp)
{
	int ret = 0;
	long offset;

	if((offset = ftell(fp)) == -1)
		ret = _elf_error("ftell", 1); /* FIXME */
	else if(_exit_64_phdr(fp) != 0 || _exit_64_shdr(fp, offset) != 0
			|| _exit_64_shdr_table(fp, offset) != 0)
		ret = 1;
	free(es64);
	return ret;
}

static int _exit_64_phdr(FILE * fp)
{
	Elf64_Ehdr hdr;

	if(es64_cnt == 0)
		return 0;
	if(fseek(fp, 0, SEEK_SET) != 0)
		return _elf_error("fseek", 1);
	if(fread(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error("fread", 1);
	hdr.e_shnum = es64_cnt;
	if(fseek(fp, 0, SEEK_SET) != 0)
		return _elf_error("fseek", 1);
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error("fwrite", 1);
	return 0;
}

static int _exit_64_shdr(FILE * fp, long offset)
{
	Elf64_Shdr hdr;
	int i;

	for(i = 0; i < es64_cnt; i++)
	{
		if(fseek(fp, es64[i].offset, SEEK_SET) != 0)
			return _elf_error("fseek", 1); /* FIXME */
		if(fread(&hdr, sizeof(hdr), 1, fp) != 1)
			return _elf_error("fread", 1); /* FIXME */
		if(i+1 == es64_cnt)
			hdr.sh_size = offset - es64[i].offset - sizeof(hdr);
		else
			hdr.sh_size = es64[i+1].offset - es64[i].offset
				- sizeof(hdr);
		if(fseek(fp, es64[i].offset, SEEK_SET) != 0)
			return _elf_error("fseek", 1); /* FIXME */
		if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
			return _elf_error("fwrite", 1); /* FIXME */
	}
	return 0;
}

static int _exit_64_shdr_table(FILE * fp, long offset)
{
	Elf64_Shdr hdr;

	if(fseek(fp, 0, SEEK_END) != 0)
		return _elf_error("fseek", 1); /* FIXME */
	memset(&hdr, 0, sizeof(hdr));
	hdr.sh_type = SHT_NULL;
	hdr.sh_offset = offset + sizeof(hdr);
	hdr.sh_link = SHN_UNDEF;
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error("fwrite", 1); /* FIXME */
	return 0;
}


static int _section_64(FILE * fp, char * section)
{
	ElfSection64 * p;
	Elf64_Shdr hdr;
	ElfSectionValues * esv;
	long offset;

	if((p = realloc(es64, sizeof(*es64) * (es64_cnt+1))) == NULL)
		return _elf_error("malloc", 1);
	es64 = p;
	p = &es64[es64_cnt++];
	memset(p, 0, sizeof(ElfSection64));
	esv = _section_values(section);
	memset(&hdr, 0, sizeof(hdr));
	hdr.sh_type = esv->type;
	hdr.sh_flags = esv->flags;
	if((offset = ftell(fp)) == -1)
		return _elf_error("ftell", 1); /* FIXME */
	hdr.sh_offset = offset + sizeof(hdr);
	p->offset = offset;
	hdr.sh_link = SHN_UNDEF; /* FIXME */
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error("fwrite", 1); /* FIXME */
	return 0;
}
