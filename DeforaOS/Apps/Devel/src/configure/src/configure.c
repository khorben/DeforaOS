/* configure.c */



#include <unistd.h>
extern int optind;
#include <libutils/libutils.h>


/* configure */
static int _configure_config(Config * config);
static int _configure(char const * directory)
{
	Config * config;
	int res = 0;

	if(chdir(directory) != 0)
	{
		fprintf(stderr, "%s", "configure: ");
		perror(directory);
		return 2;
	}
	if((config = config_new()) == NULL)
		return 2;
	if(config_load(config, "project.conf") == 0)
		res = _configure_config(config);
	else
	{
		res = 3;
		fprintf(stderr, "%s", "configure: could not open project file\n");
	}
	config_delete(config);
	return res;
}

static int _configure_config(Config * config)
{
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: configure [directory]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Config * config;
	int o;

	while((o = getopt(argc, argv, "h")) != -1)
	{
		switch(o)
		{
			case 'h':
			case '?':
				return _usage();
		}
	}
	if(argc - optind > 2)
		return _usage();
	return _configure(argc - optind == 1 ? argv[argc - 1] : ".");
}
