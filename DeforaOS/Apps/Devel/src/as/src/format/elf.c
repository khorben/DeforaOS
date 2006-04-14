/* format/elf.c */
/* FIXME
 * make a design decision on section update (read the ELF32 spec) */



#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "format.h"


/* variables */
typedef struct _FormatArch
{
	char * arch;
	int8_t machine;
	int8_t objsize;
	int8_t endian;
} FormatArch;

static FormatArch _elf_endian[] =
{
	{ "i386",    EM_386,	ELFCLASS32, ELFDATA2LSB },
	{ "i486",    EM_386,	ELFCLASS32, ELFDATA2LSB },
	{ "i586",    EM_386,	ELFCLASS32, ELFDATA2LSB },
	{ "i686",    EM_386,	ELFCLASS32, ELFDATA2LSB },
	{ "sparc",   EM_SPARC,	ELFCLASS32, ELFDATA2MSB },
	{ "sparc64", EM_SPARCV9,ELFCLASS64, ELFDATA2MSB },
	{ NULL,      0,		0,          0           }
};


typedef struct _SectionValues
{
	char * section;
	Elf32_Word type;
	Elf32_Word flags;
} SectionValues;

static SectionValues _elf_section[] =
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


/* Format */
static int _elf_error(char * message, int ret)
{
	fprintf(stderr, "%s", "as: ELF format: ");
	perror(message);
	return ret;
}

static FormatArch * _init_arch(char * arch);
int elf_init(FILE * fp, char * arch)
{
	Elf32_Ehdr hdr;
	FormatArch * fa;

	if((fa = _init_arch(arch)) == NULL)
		return 1;
	memset(&hdr, 0, sizeof(hdr));
	memcpy(&hdr.e_ident, ELFMAG, SELFMAG);
	hdr.e_ident[4] = fa->objsize;
	hdr.e_ident[5] = fa->endian;
	hdr.e_type = ET_REL;
	hdr.e_machine = fa->machine;
	hdr.e_version = EV_CURRENT;
	hdr.e_entry = 0;
	hdr.e_phoff = 0;
	hdr.e_shoff = 0;
	hdr.e_flags = 0;
	hdr.e_ehsize = sizeof(hdr);
	hdr.e_phentsize = 0;
	hdr.e_phnum = 0;
	hdr.e_shentsize = sizeof(Elf32_Shdr);
	hdr.e_shnum = 0;
	hdr.e_shstrndx = SHN_UNDEF;
	return fwrite(&hdr, sizeof(hdr), 1, fp) == 1 ? 0 : 1;
}

static FormatArch * _init_arch(char * arch)
{
	unsigned int i;

	for(i = 0; _elf_endian[i].arch != NULL; i++)
		if(strcmp(_elf_endian[i].arch, arch) == 0)
			return &_elf_endian[i];
	fprintf(stderr, "%s", "as: ELF format: Unsupported architecture\n");
	return NULL;
}


static int _section_update(FILE * fp, char * section);
int elf_exit(FILE * fp)
{
	_section_update(fp, NULL);
	return 0; /* FIXME */
}


static SectionValues * _section_values(char * section);
int elf_section(FILE * fp, char * section)
{
	Elf32_Shdr hdr;
	SectionValues * sv;

	if(_section_update(fp, section) != 0)
		return 1;
	memset(&hdr, 0, sizeof(hdr));
	sv = _section_values(section);
	hdr.sh_name = 0;
	hdr.sh_type = sv->type;
	hdr.sh_flags = sv->flags;
	hdr.sh_addr = 0;
	hdr.sh_offset = ftell(fp);
	hdr.sh_link = SHN_UNDEF;
	hdr.sh_info = 0;
	hdr.sh_addralign = 0;
	hdr.sh_entsize = 0;
	return fwrite(&hdr, sizeof(hdr), 1, fp) == 1 ? 0 : 1;
}

static int _section_update(FILE * fp, char * section)
{
	Elf32_Shdr hdr;
	static char * current = NULL;
	long offset = sizeof(Elf32_Ehdr);
	long size = 0;

	if(current == NULL)
	{
		if(section == NULL)
			return 0; /* FIXME mandatory sections? */
		if((offset = ftell(fp)) == -1
				|| (current = strdup(section)) == NULL)
			return 1;
		return 0;
	}
	if((size = ftell(fp)) == -1)
		return _elf_error(section, 1);
	size -= offset + sizeof(hdr);
	if(fseek(fp, offset, SEEK_SET) != 0)
		return _elf_error(section, 1); /* FIXME free memory if section == NULL */
	if(fread(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error(section, 1);
	hdr.sh_size = size;
	if(fseek(fp, offset, SEEK_SET) != 0)
		return _elf_error(section, 1); /* FIXME free memory if section == NULL */
	/* FIXME update the section structure */
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _elf_error(section, 1);
	if(fseek(fp, 0, SEEK_END) != 0)
		return _elf_error(section, 1); /* FIXME free memory if section == NULL */
	if(section == NULL)
	{
		free(current);
		current = NULL;
		return 0;
	}
	if((offset = ftell(fp)) == -1
			|| (current = strdup(section)) == NULL)
		return _elf_error(section, 1);
	return 0;
}

static SectionValues * _section_values(char * section)
{
	SectionValues * sv;
	int cmp;

	for(sv = _elf_section; sv->section != NULL; sv++)
		if((cmp = strcmp(section, sv->section)) == 0)
			return sv;
		else if(cmp < 0)
			break;
	for(sv = _elf_section; sv->section != NULL; sv++);
	return sv;
}


FormatPlugin format_plugin = {
	elf_init,
	elf_exit,
	elf_section
};
