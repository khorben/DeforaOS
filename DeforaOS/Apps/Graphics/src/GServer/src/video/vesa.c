/* video/vesa.c */



#include "video.h"
#include "vbe.h"


/* functions */
int vesa_init(void)
{
	return 0;
}


void vesa_destroy()
{
}


VideoPlugin video_plugin =
{
	vesa_init,
	vesa_destroy
};
