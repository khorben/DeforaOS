/* main.c */



#include "gedi.h"


/* main */
int main(int argc, char * argv[])
{
	GEDI * gedi;

	gtk_init(&argc, &argv);
	if((gedi = gedi_new()) == NULL)
		return 1;
	gtk_main();
	gedi_delete(gedi);
	return 0;
}
