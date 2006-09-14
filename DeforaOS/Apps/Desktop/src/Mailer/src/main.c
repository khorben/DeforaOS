/* main.c */



#include "mailer.h"


/* main */
int main(int argc, char * argv[])
{
	Mailer * mailer;

	gtk_init(&argc, &argv);
	if((mailer = mailer_new()) == NULL)
		return 2;
	gtk_main();
	mailer_delete(mailer);
	return 0;
}
