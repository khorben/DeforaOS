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
		fprintf(stderr, "%s%s", "configure: ",
				"could not open project file\n");
	}
	config_delete(config);
	return res;
}

static int _config_makefile(FILE * fp, Config * config);
static int _configure_config(Config * config)
{
	FILE * fp;
	int res = 0;

	if((fp = fopen("Makefile.new", "w")) == NULL)
	{
		fprintf(stderr, "%s", "configure: ");
		perror("Makefile.new");
		return 1;
	}
	if(_config_makefile(fp, config) != 0)
		res = 2;
	fclose(fp);
	return res;
}

static int _makefile_variables(FILE * fp, Config * config);
static int _makefile_targets(FILE * fp, Config * config);
static int _makefile_clean(FILE * fp, Config * config);
static int _config_makefile(FILE * fp, Config * config)
{
	int res = 0;

	res += _makefile_variables(fp, config);
	fprintf(fp, "\n");
	res += _makefile_targets(fp, config);
	fprintf(fp, "\n");
	res += _makefile_clean(fp, config);
	return res;
}

static int _variables_subdirs(FILE * fp, Config * config);
static int _variables_target(FILE * fp, Config * config);
static int _variables_cflags(FILE * fp, Config * config);
static int _variables_ldflags(FILE * fp, Config * config);
static int _variables_misc(FILE * fp, Config * config);
static int _makefile_variables(FILE * fp, Config * config)
{
	int res = 0;

	res += _variables_subdirs(fp, config);
	res += _variables_target(fp, config);
	res += _variables_cflags(fp, config);
	res += _variables_ldflags(fp, config);
	res += _variables_misc(fp, config);
	return res;
}

static int _variables_subdirs(FILE * fp, Config * config)
{
	char * subdirs;
	char * cur;

	if((subdirs = config_get(config, "", "subdirs")) == NULL
			|| *subdirs == '\0')
		return 0;
	fprintf(fp, "%s", "SUBDIRS\t=");
	for(cur = subdirs; *subdirs != '\0'; subdirs++)
	{
		if(*subdirs != ',')
			continue;
		*subdirs = '\0';
		fprintf(fp, " %s", cur);
		*subdirs = ',';
		cur = subdirs + 1;
	}
	fprintf(fp, " %s\n", cur);
	return 0;
}

static int _variables_target(FILE * fp, Config * config)
{
	char * targets;
	char * cur;

	if((targets = config_get(config, "", "targets")) == NULL
			|| *targets == '\0')
	{
		/* FIXME */
		return 1;
	}
	fprintf(fp, "%s", "TARGETS\t=");
	for(cur = targets; *targets != '\0'; targets++)
	{
		if(*targets != ',')
			continue;
		*targets = '\0';
		fprintf(fp, " %s", cur);
		*targets = ',';
		cur = targets + 1;
	}
	fprintf(fp, " %s\n", cur);
	return 0;
}

static int _variables_cflags(FILE * fp, Config * config)
{
	char * cflags;

	if((cflags = config_get(config, "", "cflags_force")) != NULL
			&& *cflags != '\0')
		fprintf(fp, "%s%s%s", "CFLAGSF\t= ", cflags, "\n");
	if((cflags = config_get(config, "", "cflags")) != NULL
			&& *cflags != '\0')
		fprintf(fp, "%s%s%s", "CFLAGS\t= ", cflags, "\n");
	return 0;
}

static int _variables_ldflags(FILE * fp, Config * config)
{
	char * ldflags;

	if((ldflags = config_get(config, "", "ldflags_force")) != NULL
			&& *ldflags != '\0')
		fprintf(fp, "%s%s%s", "LDFLAGSF= ", ldflags, "\n");
	if((ldflags = config_get(config, "", "ldflags")) != NULL
			&& *ldflags != '\0')
		fprintf(fp, "%s%s%s", "LDFLAGS\t= ", ldflags, "\n");
	return 0;
}

static int _variables_misc(FILE * fp, Config * config)
{
	fprintf(fp, "%s", "CC\t= cc\nRM\t= rm -f\n");
	return 0;
}

static int _targets_all(FILE * fp, Config * config);
static int _makefile_targets(FILE * fp, Config * config)
{
	int res = 0;

	res += _targets_all(fp, config);
	return res;
}

static void _target_objs(FILE * fp, Config * config, char * target);
static int _targets_all(FILE * fp, Config * config)
{
	char * targets;
	char * cur;

	fprintf(fp, "%s", "\nall:");
	if((targets = config_get(config, "", "targets")) == NULL
			|| *targets == '\0')
	{
		fprintf(fp, "\n");
		return 0;
	}
	fprintf(fp, "%s", " $(TARGETS)\n\n");
	for(cur = targets; *targets != '\0'; targets++)
	{
		if(*targets != ',')
			continue;
		*targets = '\0';
		_target_objs(fp, config, cur);
		*targets = ',';
		cur = targets + 1;
	}
	_target_objs(fp, config, cur);
	return 0;
}

static void _obj_print(FILE * fp, char * obj);
static void _objs_handlers(FILE * fp, Config * config, char * target);
static void _target_objs(FILE * fp, Config * config, char * target)
{
	char * sources;
	char * cur;

	if((sources = config_get(config, target, "sources")) == NULL
			|| *sources == '\0')
		return;
	fprintf(fp, "%s_OBJS=", target);
	for(cur = sources; *sources != '\0'; sources++)
	{
		if(*sources != ',')
			continue;
		*sources = '\0';
		fprintf(fp, "%s", " ");
		_obj_print(fp, cur);
		*sources = ',';
		cur = sources + 1;
	}
	fprintf(fp, "%s", " ");
	_obj_print(fp, cur);
	fprintf(fp, "\n%s%s%s%s%s%s%s%s%s", target, ": $(", target, "_OBJS)\n",
			"\t$(CC) $(LDFLAGSF) $(LDFLAGS) -o ", target, " $(",
			target, "_OBJS)\n\n");
	_objs_handlers(fp, config, target);
}

static void _obj_print(FILE * fp, char * obj)
{
	int len;

	for(len = strlen(obj) - 1; len >= 0 && obj[len] != '.'; len--);
	if(strcmp(&obj[len+1], "c") == 0)
	{
		obj[len+1] = 'o';
		fprintf(fp, "%s", obj);
		obj[len+1] = 'c';
		return;
	}
	fprintf(stderr, "%s%s%s", "configure: ", obj,
			": unknown source type\n");
}

static void _handler_print(FILE * fp, char * source);
static void _objs_handlers(FILE * fp, Config * config, char * target)
{
	char * sources;
	char * cur;

	if((sources = config_get(config, target, "sources")) == NULL
			|| *sources == '\0')
		return;
	for(cur = sources; *sources != '\0'; sources++)
	{
		if(*sources != ',')
			continue;
		*sources = '\0';
		_handler_print(fp, cur);
		*sources = ',';
		cur = sources + 1;
	}
	_handler_print(fp, cur);
}

static void _handler_print(FILE * fp, char * source)
{
	_obj_print(fp, source);
	fprintf(fp, "%s%s%s%s%s", ": ", source,
			"\n\t$(CC) $(CFLAGSF) $(CFLAGS) -c ", source, "\n\n");
}

static void _clean_targets_objs(FILE * fp, Config * config);
static int _makefile_clean(FILE * fp, Config * config)
{
	fprintf(fp, "%s", "clean:\n\t$(RM)");
	if(config_get(config, "", "targets") != NULL)
		_clean_targets_objs(fp, config);
	fprintf(fp, "\n");
	fprintf(fp, "%s", "\ndistclean: clean\n\t$(RM)");
	if(config_get(config, "", "targets") != NULL)
		fprintf(fp, "%s", " $(TARGETS)");
	fprintf(fp, "\n");
	return 0;
}

static void _clean_targets_objs(FILE * fp, Config * config)
{
	char * targets;
	char * cur;

	if((targets = config_get(config, "", "targets")) == NULL
			|| *targets == '\0')
		return;
	for(cur = targets; *targets != '\0'; targets++)
	{
		if(*targets != ',')
			continue;
		*targets = '\0';
		fprintf(fp, "%s%s%s", " $(", cur, "_OBJS)");
		*targets = ',';
		cur = targets + 1;
	}
	fprintf(fp, "%s%s%s", " $(", cur, "_OBJS)");
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
