/* video/vbe.h */



#ifndef __VIDEO_VBE_H
# define __VIDEO_VBE_H

#include <stdint.h>


/* types */
/* FIXME only present useful sections and do the selection in vbe.c */
typedef struct _VbeInfo
{
	char vbesignature[4];
	int16_t vbeversion;
	int32_t * oemstringptr;
	char capabilities[4];
	int32_t videomodeptr;
	int16_t totalmemory;
	/* VBE 2.0+ */
	int16_t oemsoftwarerev;
	int32_t oemvendornameptr;
	int32_t oemproductnameptr;
	int32_t oemproductrevptr;
	char reserved[222];
	char oemdata[256];
} VbeInfo;


/* functions */
int vbe_info(VbeInfo * buf);

#endif /* !__VIDEO_VBE_H */
