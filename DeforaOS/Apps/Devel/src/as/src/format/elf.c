/* format/elf.c */
/* FIXME
 * make a design decision on section update (read the ELF32 spec) */



#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "format.h"

/* FIXME */
#define as_error(message, ret) fprintf(stderr, "%d\n", (ret))


/* types */
typedef struct _FormatArch
{
	char * arch;
	int8_t machine;
	int8_t objsize;
	int8_t endian;
} FormatArch;


/* variables */
static FormatArch _elf_endian[] =
{
	{ "i386",    EM_386,	ELFCLASS32, ELFDATA2LSB },
	{ "i486",    EM_386,	ELFCLASS32, ELFDATA2LSB },
	{ "i586",    EM_386,	ELFCLASS32, ELFDATA2LSB },
	{ "i686",    EM_386,	ELFCLASS32, ELFDATA2LSB },
	{ "sparc",   EM_SPARC,	ELFCLASS32, ELFDATA2LSB },
	{ "sparc64", EM_SPARCV9,ELFCLASS64, ELFDATA2MSB },
	{ NULL,      0,		0,          0           }
};


/* Format */
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
	hdr.e_version = EV_CURRENT;	/* FIXME */
	hdr.e_entry = 0;		/* FIXME may be redefined later */
	hdr.e_phoff = 0;		/* FIXME may be redefined later */
	hdr.e_shoff = 0;		/* FIXME may be redefined later */
	hdr.e_flags = EF_CPU32;
	hdr.e_ehsize = sizeof(hdr);
	hdr.e_phentsize = 4;		/* FIXME */
	hdr.e_phnum = 0;
	hdr.e_shentsize = 4;		/* FIXME */
	hdr.e_shnum = 0;
	hdr.e_shstrndx = SHN_UNDEF;	/* FIXME */
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


int elf_section(FILE * fp, char * section)
{
	Elf32_Shdr hdr;

	if(_section_update(fp, section) != 0)
		return 1;
	memset(&hdr, 0, sizeof(hdr));
	hdr.sh_type = SHT_PROGBITS;	/* FIXME */
	hdr.sh_flags = SHF_EXECINSTR;	/* FIXME */
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
		return as_error(section, 1);
	size -= offset + sizeof(hdr);
	if(fseek(fp, offset, SEEK_SET) != 0)
		return as_error(section, 1); /* FIXME free memory if section == NULL */
	if(fread(&hdr, sizeof(hdr), 1, fp) != 1)
		return as_error(section, 1);
	hdr.sh_size = size;
	if(fseek(fp, offset, SEEK_SET) != 0)
		return as_error(section, 1); /* FIXME free memory if section == NULL */
	/* FIXME update the section structure */
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return as_error(section, 1);
	if(fseek(fp, 0, SEEK_END) != 0)
		return 1; /* FIXME free memory if section == NULL */
	if(section == NULL)
	{
		free(current);
		current = NULL;
		return 0;
	}
	if((offset = ftell(fp)) == -1
			|| (current = strdup(section)) == NULL)
		return 1;
	return 0;
}


FormatPlugin format_plugin = {
	elf_init,
	elf_exit,
	elf_section
};
