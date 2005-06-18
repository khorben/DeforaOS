/* format/elf.c */



#include <elf.h>
#include <string.h>
#include "format.h"



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
	memcpy(&hdr.e_ident, ELFMAG, SELFMAG);
	hdr.e_ident[4] = fa->objsize;
	hdr.e_ident[5] = fa->endian;
	hdr.e_type = ET_REL;		/* FIXME */
	hdr.e_machine = fa->machine;
	hdr.e_version = EV_CURRENT;	/* FIXME */
	hdr.e_flags = EF_CPU32;		/* FIXME */
	return fwrite(&hdr, sizeof(hdr), 1, fp) == 1 ? 0 : 1;
}

static FormatArch * _init_arch(char * arch)
{
	unsigned int i;

	for(i = 0; _elf_endian[i].arch != NULL; i++)
		if(strcmp(_elf_endian[i].arch, arch) == 0)
			return &_elf_endian[i];
	fprintf(stderr, "%s", "as: format: Unknown architecture\n");
	return NULL;
}


int elf_exit(void)
{
	return 0; /* FIXME */
}


FormatPlugin format_plugin = {
	elf_init,
	elf_exit
};
