/* configure.c */



#include <unistd.h>
extern int optind;
#include <stdio.h>
#include <string.h>
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

static int _configure_target(Config * config, char const * target);
static int _configure_config(Config * config)
{
	char * targets;
	char * cur;
	
	if((targets = config_get(config, "", "targets")) == NULL
			|| *targets == '\0')
	{
		fprintf(stderr, "%s", "configure: no targets found\n");
		return 1;
	}
	for(cur = targets; *targets; targets++)
	{
		if(*targets != ',')
			continue;
		*targets = '\0';
		_configure_target(config, cur);
		cur = targets + 1;
	}
	_configure_target(config, cur);
	return 0;
}

static int _configure_binary(Config * config, char const * target);
static int _configure_library(Config * config, char const * target);
static int _configure_target(Config * config, char const * target)
{
	char * type;

	if((type = config_get(config, target, "type")) == NULL)
	{
		fprintf(stderr, "%s%s%s", "configure: target \"", target,
				"\" has empty type\n");
		return 1;
	}
	if(strcmp("binary", type) == 0)
		return _configure_binary(config, target);
	if(strcmp("library", type) == 0)
		return _configure_library(config, target);
	fprintf(stderr, "%s%s%s%s%s", "configure: target \"", target,
			"\" has unknown type \"", type, "\"\n");
	return 1;
}

static int _configure_binary(Config * config, char const * target)
{
	char * sources;
	char * cur;

	if((sources = config_get(config, target, "sources")) == NULL
			|| *sources == '\0')
	{
		fprintf(stderr, "%s%s%s", "configure: target \"", target,
				"\" has empty type\n");
		return 1;
	}
	for(cur = sources; *sources; sources++)
	{
		if(*sources != ',')
			continue;
		fprintf(stderr, "%s%s%s", "configure: would add source \"",
				cur, "\"\n");
		cur = sources + 1;
	}
	fprintf(stderr, "%s%s%s", "configure: would add source \"",
			cur, "\"\n");
	return 0;
}

static int _configure_library(Config * config, char const * target)
{
	fprintf(stderr, "%s", "configure: libraries are not implemented yet\n");
	return 1;
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
