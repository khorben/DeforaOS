/* format/elf.c */



#include <elf.h>
#include <string.h>
#include "format.h"


/* Format */
int elf_init(FILE * fp)
{
	Elf32_Ehdr hdr;

	memcpy(&hdr.e_ident, ELFMAG, SELFMAG);
	hdr.e_ident[4] = ELFCLASS32;	/* FIXME */
	hdr.e_ident[5] = ELFDATA2LSB;	/* FIXME */
	hdr.e_type = ET_REL;		/* FIXME */
	hdr.e_machine = EM_386;		/* FIXME */
	hdr.e_version = EV_CURRENT;	/* FIXME */
	hdr.e_flags = EF_CPU32;		/* FIXME */
	return fwrite(&hdr, sizeof(hdr), 1, fp) == 1 ? 0 : 1;
}


int elf_exit(void)
{
	return 0; /* FIXME */
}


FormatPlugin format_plugin = {
	elf_init,
	elf_exit
};
