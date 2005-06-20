/* format/flat.c */



#include "format.h"


/* Format */
int flat_init(FILE * fp, char * arch)
{
	return 0;
}


int flat_exit(void)
{
	return 0;
}


FormatPlugin format_plugin = {
	flat_init,
	flat_exit
};
