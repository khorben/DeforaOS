/* configure.c */



#include <libutils/libutils.h>


/* usage */
static int usage(void)
{
	fprintf(stderr, "%s", "Usage: configure\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Config * config;

	if(argc != 1)
		return usage();
	if((config = config_new("project.conf")) == NULL)
		return 2;
	config_delete(config);
	return 0;
}
