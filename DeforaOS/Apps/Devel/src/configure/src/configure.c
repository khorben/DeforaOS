/* configure.c */



#include <System.h>
#include <unistd.h>
#include <stdio.h>


/* configure */
/* PRE
 * POST		specified directory has been setup */
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
		res = 2;
		fprintf(stderr, "%s%s%s", "configure: ", directory,
				": Could not open project file\n");
	}
	config_delete(config);
	return res;
}

static int _config_makefile(FILE * fp, Config * config);
static int _configure_config(Config * config)
{
	FILE * fp;
	int res = 0;

	if((fp = fopen("Makefile", "w")) == NULL)
	{
		fprintf(stderr, "%s", "configure: ");
		perror("Makefile");
		return 1;
	}
	if(_config_makefile(fp, config) != 0)
		res = 3;
	fclose(fp);
	return res;
}

static int _makefile_subdirs(Config * config);
static int _makefile_variables(FILE * fp, Config * config);
static int _makefile_targets(FILE * fp, Config * config);
static int _makefile_clean(FILE * fp, Config * config);
static int _config_makefile(FILE * fp, Config * config)
{
	int res = 0;

	res += _makefile_subdirs(config);
	res += _makefile_variables(fp, config);
	fprintf(fp, "\n");
	res += _makefile_targets(fp, config);
	fprintf(fp, "\n");
	res += _makefile_clean(fp, config);
	return res;
}

static int _subdir_configure(char const * subdirectory);
static int _makefile_subdirs(Config * config)
{
	char * subdirs;
	char * cur;
	int res = 0;

	if((subdirs = config_get(config, "", "subdirs")) == NULL
			|| *subdirs == '\0')
		return 0;
	for(cur = subdirs; *subdirs != '\0'; subdirs++)
	{
		if(*subdirs != ',')
			continue;
		*subdirs = '\0';
		res += _subdir_configure(cur);
		*subdirs = ',';
		cur = subdirs + 1;
	}
	res += _subdir_configure(cur);
	return res;
}

static int _subdir_configure(char const * subdir)
{
	if(string_find(subdir, "/") == NULL && string_compare(subdir, ".")
			&& string_compare(subdir, ".."))
	{
		if(_configure(subdir) != 2)
			chdir("..");
		return 0;
	}
	fprintf(stderr, "%s%s%s", "configure: ", subdir,
			": Invalid subdirectory\n");
	return 1;
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

static void _target_print(FILE * fp, Config * config, char * target);
static int _variables_target(FILE * fp, Config * config)
{
	char * targets;
	char * cur;

	if((targets = config_get(config, "", "targets")) == NULL
			|| *targets == '\0')
		return 1;
	fprintf(fp, "%s", "TARGETS\t=");
	for(cur = targets; *targets != '\0'; targets++)
	{
		if(*targets != ',')
			continue;
		*targets = '\0';
		_target_print(fp, config, cur);
		*targets = ',';
		cur = targets + 1;
	}
	_target_print(fp, config, cur);
	fputc('\n', fp);
	return 0;
}

static void _target_print(FILE * fp, Config * config, char * target)
{
	char * p;

	if((p = config_get(config, target, "type")) != NULL
			&& string_compare(p, "library") == 0)
		fprintf(fp, " %s.a %s.so", target, target);
	else
		fprintf(fp, " %s", target);
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
	char * targets;

	if((targets = config_get(config, "", "targets")) != NULL
			&& *targets != '\0')
		fprintf(fp, "%s%s%s", "CC\t= cc\nAR\t= ar rc\n",
				"RANLIB\t= ranlib\nLD\t= ld -shared\n",
				"RM\t= rm -f\n");
	return 0;
}

static int _targets_all(FILE * fp, Config * config);
static int _makefile_targets(FILE * fp, Config * config)
{
	char * subdirs;
	char * targets;

	fprintf(fp, "%s", "\nall:");
	if((subdirs = config_get(config, "", "subdirs")) != NULL
			&& *subdirs != '\0')
		fprintf(fp, "%s", " subdirs");
	if((targets = config_get(config, "", "targets")) != NULL
			&& *targets != '\0')
		fprintf(fp, "%s", " $(TARGETS)");
	fprintf(fp, "%s", "\n\n");
	if(subdirs != NULL && *subdirs != '\0')
		fprintf(fp, "%s%s%s", "subdirs:\n",
				"\t@for i in $(SUBDIRS); do ",
				"(cd $$i && $(MAKE)) || exit; done\n\n");
	if(targets == NULL)
		return 0;
	return _targets_all(fp, config);
}

static void _target_objs(FILE * fp, Config * config, char * target);
static int _targets_all(FILE * fp, Config * config)
{
	char * targets;
	char * cur;

	if((targets = config_get(config, "", "targets")) == NULL
			|| *targets == '\0')
		return 0;
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
static void _target_link(FILE * fp, Config * config, char * target);
static void _objs_handlers(FILE * fp, Config * config, char * target);
static void _target_objs(FILE * fp, Config * config, char * target)
{
	char * sources;
	char * cur;

	if((sources = config_get(config, target, "sources")) == NULL
			|| *sources == '\0')
	{
		fprintf(stderr, "%s%s%s", "configure: ", target,
				": Undefined target\n");
		return;
	}
	fprintf(fp, "%s%s", target, "_OBJS=");
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
	fprintf(fp, "\n%s%s", target, "_CFLAGS=$(CFLAGSF)");
	cur = config_get(config, target, "cflags");
	fprintf(fp, "%s%s%s", cur != NULL ? " " : "", cur != NULL ? cur : "",
			" $(CFLAGS)\n");
	_target_link(fp, config, target);
	_objs_handlers(fp, config, target);
}

static void _obj_print(FILE * fp, char * obj)
{
	int len;

	for(len = string_length(obj) - 1; len >= 0 && obj[len] != '.'; len--);
	if(string_compare(&obj[len+1], "c") == 0)
	{
		obj[len+1] = 'o';
		fprintf(fp, "%s", obj);
		obj[len+1] = 'c';
	}
	else if(string_compare(&obj[len+1], "S") == 0)
	{
		obj[len+1] = 'o';
		fprintf(fp, "%s", obj);
		obj[len+1] = 'S';
	}
	else
		fprintf(stderr, "%s%s%s", "configure: ", obj,
			": Unknown source type\n");
}

static void _target_link(FILE * fp, Config * config, char * target)
{
	char * type;
	char * p;

	if((type = config_get(config, target, "type")) == NULL)
	{
		fprintf(stderr, "%s%s%s", "configure: ", target,
				": Empty type\n");
		return;
	}
	if(string_compare("binary", type) == 0)
	{
		fprintf(fp, "%s%s%s%s%s", target, ": $(", target,
				"_OBJS)\n", "\t$(CC) $(LDFLAGSF) $(LDFLAGS) ");
		if((p = config_get(config, target, "ldflags")) != NULL)
			fprintf(fp, "%s%s", p, " ");
		fprintf(fp, "%s%s%s%s%s", "-o ", target,
				" $(", target, "_OBJS)\n\n");
	}
	else if(string_compare("library", type) == 0)
		fprintf(fp, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
				target, ".a: $(", target, "_OBJS)\n\t$(AR) ",
				target,	".a $(", target, "_OBJS)\n\t$(RANLIB) ",
				target,	".a\n\n", target, ".so: $(", target,
				"_OBJS)\n\t$(LD) -o ", target, ".so $(",
				target, "_OBJS)\n\n");
	else
		fprintf(stderr, "%s%s%s%s%s", "configure: ", target,
				": Unknown type \"", type, "\"\n");
}

static void _handler_print(FILE * fp, char * target, char * source);
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
		_handler_print(fp, target, cur);
		*sources = ',';
		cur = sources + 1;
	}
	_handler_print(fp, target, cur);
}

static void _handler_print(FILE * fp, char * target, char * source)
{
	int len;

	_obj_print(fp, source);
	fprintf(fp, "%s%s%s%s%s%s%s", ": ", source, "\n\t$(CC) $(", target,
			"_CFLAGS)", " -c ", source);
	if(string_find(source, "/") != NULL)
	{
		len = string_length(source);
		source[len-1] = 'o';
		fprintf(fp, "%s%s", " -o ", source);
		source[len-1] = 'c';
	}
	fprintf(fp, "%s", "\n\n");
}

static void _clean_targets_objs(FILE * fp, Config * config);
static int _makefile_clean(FILE * fp, Config * config)
{
	char * subdirs;

	fprintf(fp, "%s", "clean:\n");
	if((subdirs = config_get(config, "", "subdirs")) != NULL)
		fprintf(fp, "%s%s", "\t@for i in $(SUBDIRS); ",
				"do (cd $$i && $(MAKE) clean) || exit; done\n");
	if(config_get(config, "", "targets") != NULL)
	{
		fprintf(fp, "%s", "\t$(RM)");
		_clean_targets_objs(fp, config);
		fprintf(fp, "\n");
	}
	fprintf(fp, "%s", "\ndistclean: clean\n");
	if(subdirs != NULL)
		fprintf(fp, "%s%s", "\t@for i in $(SUBDIRS); ",
				"do (cd $$i && $(MAKE) distclean) || exit; done\n");
	if(config_get(config, "", "targets") != NULL)
		fprintf(fp, "%s", "\t$(RM) $(TARGETS)\n");
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
	if(argc - optind > 1)
		return _usage();
	return _configure(argc - optind == 1 ? argv[argc - 1] : ".");
}
