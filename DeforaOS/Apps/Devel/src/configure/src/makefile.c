/* makefile.c */



#include <System.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "configure.h"

ARRAY(Config *, config);


/* functions */
static int _makefile_write(Configure * configure, Config * config, FILE * fp,
		configArray * ca, int from, int to);
int makefile(Configure * configure, Config * config, String * directory,
		configArray * ca, int from, int to)
{
	String * makefile;
	FILE * fp = NULL;
	int ret = 0;

	makefile = string_new(directory);
	string_append(&makefile, "/");
	string_append(&makefile, MAKEFILE);
	if(!(configure->prefs->flags & PREFS_n)
			&& (fp = fopen(makefile, "w")) == NULL)
		ret = configure_error(makefile, 1);
	else
	{
		if(configure->prefs->flags & PREFS_v)
			printf("%s%s%s%s%s", "Creating ", MAKEFILE, " in ",
					directory, "\n");
		ret |= _makefile_write(configure, config, fp, ca, from, to);
		if(fp != NULL)
			fclose(fp);
	}
	string_delete(makefile);
	return ret;
}

static int _write_variables(Configure * configure, Config * config, FILE * fp);
static int _write_targets(Prefs * prefs, Config * config, FILE * fp);
static int _write_objects(Prefs * prefs, Config * config, FILE * fp);
static int _write_clean(Prefs * prefs, Config * config, FILE * fp);
static int _write_distclean(Prefs * prefs, Config * config, FILE * fp);
static int _write_dist(Prefs * prefs, Config * config, FILE * fp,
		configArray * ca, int from, int to);
static int _write_install(Prefs * prefs, Config * config, FILE * fp);
static int _write_uninstall(Prefs * prefs, Config * config, FILE * fp);
static int _makefile_write(Configure * configure, Config * config, FILE * fp,
		configArray * ca, int from, int to)
{
	if(_write_variables(configure, config, fp) != 0
			|| _write_targets(configure->prefs, config, fp) != 0
			|| _write_objects(configure->prefs, config, fp) != 0
			|| _write_clean(configure->prefs, config, fp) != 0
			|| _write_distclean(configure->prefs, config, fp) != 0
			|| _write_dist(configure->prefs, config, fp, ca, from,
				to) != 0
			|| _write_install(configure->prefs, config, fp) != 0
			|| _write_uninstall(configure->prefs, config, fp) != 0)
		return 1;
	if(!(configure->prefs->flags & PREFS_n))
		fprintf(fp, "%s%s%s", "\n.PHONY: all",
				config_get(config, "", "subdirs") != NULL
				? " subdirs" : "",
				" clean distclean install uninstall\n");
	return 0;
}

static int _variables_package(Prefs * prefs, Config * config, FILE * fp,
		String const * directory);
static int _variables_print(Prefs * prefs, Config * config, FILE * fp,
		char const * input, char const * output);
static int _variables_targets(Prefs * prefs, Config * config, FILE * fp);
static int _variables_executables(Configure * configure, Config * config,
		FILE * fp);
static int _write_variables(Configure * configure, Config * config, FILE * fp)
{
	String const * directory = config_get(config, "", "directory");
	int ret = 0;

	ret |= _variables_package(configure->prefs, config, fp, directory);
	ret |= _variables_print(configure->prefs, config, fp, "subdirs",
			"SUBDIRS");
	ret |= _variables_targets(configure->prefs, config, fp);
	ret |= _variables_executables(configure, config, fp);
	if(!(configure->prefs->flags & PREFS_n))
		fputc('\n', fp);
	return ret;
}

static int _variables_package(Prefs * prefs, Config * config, FILE * fp,
		String const * directory)
{
	String * package;
	String * version;

	if((package = config_get(config, "", "package")) == NULL)
		return 0;
	if(prefs->flags & PREFS_v)
		printf("%s%s", "Package: ", package);
	if((version = config_get(config, "", "version")) == NULL)
	{
		if(prefs->flags & PREFS_v)
			fputc('\n', stdout);
		fprintf(stderr, "%s%s%s", "configure: ", directory,
				": \"package\" needs \"version\"\n");
		return 1;
	}
	if(prefs->flags & PREFS_v)
		printf("%s%s%s", " ", version, "\n");
	if(fp != NULL)
		fprintf(fp, "%s%s%s%s%s", "PACKAGE\t= ", package,
				"\nVERSION\t= ", version, "\n");
	return 0;
}

static int _variables_print(Prefs * prefs, Config * config, FILE * fp,
		char const * input, char const * output)
{
	String * prints;
	int i;
	char c;

	if(prefs->flags & PREFS_n)
		return 0;
	if((prints = config_get(config, "", input)) == NULL)
		return 0;
	fprintf(fp, "%s%s", output, "\t=");
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		fprintf(fp, " %s", prints);
		if(c == '\0')
			break;
		prints[i] = c;
		prints+=i+1;
		i = 0;
	}
	fputc('\n', fp);
	return 0;
}

static int _variables_targets(Prefs * prefs, Config * config, FILE * fp)
{
	String * prints;
	int i;
	char c;
	String * type;

	if(prefs->flags & PREFS_n)
		return 0;
	if((prints = config_get(config, "", "targets")) == NULL)
		return 0;
	fprintf(fp, "%s%s", "TARGETS", "\t=");
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		if((type = config_get(config, prints, "type")) == NULL)
			fprintf(fp, " %s", prints);
		else
			switch(enum_string(TT_LAST, sTargetType, type))
			{
				case TT_BINARY:
				case TT_OBJECT:
				case TT_UNKNOWN:
					fprintf(fp, " %s", prints);
					break;
				case TT_LIBRARY:
					fprintf(fp, " %s%s%s%s", prints, ".a ",
							prints, ".so");
					break;
			}
		if(c == '\0')
			break;
		prints[i] = c;
		prints+=i+1;
		i = 0;
	}
	fputc('\n', fp);
	return 0;
}

static int _executables_variables(Configure * configure, Config * config,
		FILE * fp, String * target);
static int _variables_executables(Configure * configure, Config * config,
		FILE * fp)
{
	String * targets;
	int i;
	char c;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	if((targets = config_get(config, "", "targets")) != NULL)
	{
		for(i = 0;; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			_executables_variables(configure, config, fp, targets);
			if(c == '\0')
				break;
			targets[i] = c;
			targets+=i+1;
			i = 0;
		}
		fprintf(fp, "%s", "RM\t= rm -f\n");
	}
	if(config_get(config, "", "package"))
		fprintf(fp, "%s", "LN\t= ln -sf\nTAR\t= tar -czvf\n");
	if(targets != NULL)
	{
		fprintf(fp, "%s", "MKDIR\t= mkdir -p\n");
		fprintf(fp, "%s", "INSTALL\t= install\n");
	}
	return 0;
}

static void _variables_binary(Configure * configure, Config * config, FILE * fp,
		char * done);
static void _variables_library(Configure * configure, Config * config,
		FILE * fp, char * done);
static int _executables_variables(Configure * configure, Config * config,
		FILE * fp, String * target)
{
	static Config * flag = NULL;
	String * type;
	char done[TT_LAST]; /* FIXME even better if'd be variable by variable */
	TargetType tt;

	if(flag != config)
	{
		flag = config;
		memset(done, 0, sizeof(done));
	}
	if((type = config_get(config, target, "type")) == NULL)
		return 0;
	if(done[(tt = enum_string(TT_LAST, sTargetType, type))])
		return 0;
	switch(tt)
	{
		case TT_BINARY:
			_variables_binary(configure, config, fp, done);
			break;
		case TT_LIBRARY:
			_variables_library(configure, config, fp, done);
			break;
		case TT_OBJECT:
		case TT_UNKNOWN:
			break;
	}
	done[tt] = 1;
	return 0;
}

static void _binary_ldflags(Configure * configure, Config * config, FILE * fp,
		String const * ldflags);
static void _variables_binary(Configure * configure, Config * config, FILE * fp,
		char * done)
{
	String const * p;

	/* FIXME path given from user or autodetected */
	if(!done[TT_LIBRARY])
	{
		fprintf(fp, "%s%s\n", "PREFIX\t= ", configure->prefs->prefix);
		fprintf(fp, "%s%s\n", "DESTDIR\t= ", configure->prefs->destdir);
	}
	if(configure->prefs->bindir[0] == '/')
		fprintf(fp, "%s%s\n", "BINDIR\t= ", configure->prefs->bindir);
	else
		fprintf(fp, "%s%s\n", "BINDIR\t= $(PREFIX)/",
				configure->prefs->bindir);
	if(configure->prefs->includedir[0] == '/')
		fprintf(fp, "%s%s\n", "INCLUDEDIR= ",
				configure->prefs->includedir);
	else
		fprintf(fp, "%s%s\n", "INCLUDEDIR= $(PREFIX)/",
				configure->prefs->includedir);
	if(!done[TT_LIBRARY])
	{
		fprintf(fp, "%s", "CC\t= cc\n");
		if((p = config_get(config, "", "cflags_force"))
				!= NULL)
		{
			fprintf(fp, "%s%s", "CFLAGSF\t= ", p);
			if(configure->os == HO_GNU_LINUX
					&& string_find(p, "-ansi"))
				fprintf(fp, "%s", " -D_GNU_SOURCE");
			fputc('\n', fp);
		}
		if((p = config_get(config, "", "cflags"))
				!= NULL)
			fprintf(fp, "%s%s%s", "CFLAGS\t= ", p,
					"\n");
	}
	if((p = config_get(config, "", "ldflags_force"))
			!= NULL)
	{
		fprintf(fp, "%s", "LDFLAGSF= ");
		_binary_ldflags(configure, config, fp, p);
	}
	if((p = config_get(config, "", "ldflags")) != NULL)
	{
		fprintf(fp, "%s", "LDFLAGS\t= ");
		_binary_ldflags(configure, config, fp, p);
	}
}

static void _binary_ldflags(Configure * configure, Config * config, FILE * fp,
		String const * ldflags)
{
	/* FIXME remove -l dl and -l crypt on BSD, check on Solaris etc */
	char * libs_gnu[] = { "socket", NULL };
	char * libs_bsd[] = { "dl", "socket", NULL };
	char * libs_sunos[] = { "dl", NULL };
	char buf[10];
	char ** libs;
	char * p;
	char * q;
	int i;

	if((p = string_new(ldflags)) == NULL)
	{
		fprintf(fp, "%s%s", ldflags, "\n");
		return;
	}
	switch(configure->os)
	{
		case HO_GNU_LINUX:
			libs = libs_gnu;
			break;
		case HO_FREEBSD:
		case HO_NETBSD:
		case HO_OPENBSD:
			libs = libs_bsd;
			break;
		case HO_SUNOS:
			libs = libs_sunos;
			break;
		default:
			libs = libs_gnu;
			break;
	}
	for(i = 0; libs[i] != NULL; i++)
	{
		snprintf(buf, sizeof(buf), "-l %s", libs[i]);
		if((q = string_find(p, buf)) == NULL)
			continue;
		memmove(q, q + strlen(buf), strlen(q) - strlen(buf) + 1);
	}
	fprintf(fp, "%s%s", p, "\n");
	free(p);
}

static void _variables_library(Configure * configure, Config * config,
		FILE * fp, char * done)
{
	String * p;

	/* FIXME path given from user or autodetected */
	if(!done[TT_LIBRARY])
	{
		fprintf(fp, "%s", "PREFIX\t= /usr/local\n");
		fprintf(fp, "%s", "DESTDIR\t=\n");
	}
	fprintf(fp, "%s", "LIBDIR\t= $(PREFIX)/lib\n");
	if(!done[TT_BINARY])
	{
		fprintf(fp, "%s", "CC\t= cc\n");
		if((p = config_get(config, "", "cflags_force"))
				!= NULL)
		{
			fprintf(fp, "%s%s", "CFLAGSF\t= ", p);
			if(configure->os == HO_GNU_LINUX
					&& string_find(p, "-ansi"))
				fprintf(fp, "%s", " -D_GNU_SOURCE");
			fputc('\n', fp);
		}
		if((p = config_get(config, "", "cflags"))
				!= NULL)
			fprintf(fp, "%s%s%s", "CFLAGS\t= ", p,
					"\n");
	}
	fprintf(fp, "%s", "AR\t= ar -rc\n");
	fprintf(fp, "%s", "RANLIB\t= ranlib\n");
	fprintf(fp, "%s", "LD\t= ld -shared\n");
}

static int _targets_all(Prefs * prefs, Config * config, FILE * fp);
static int _targets_subdirs(Prefs * prefs, Config * config, FILE * fp);
static int _targets_target(Prefs * prefs, Config * config, FILE * fp,
		String * target);
static int _write_targets(Prefs * prefs, Config * config, FILE * fp)
{
	char * targets = config_get(config, "", "targets");
	char c;
	int i;
	int ret = 0;

	if(_targets_all(prefs, config, fp) != 0
			|| _targets_subdirs(prefs, config, fp) != 0)
		return 1;
	if(targets == NULL)
		return 0;
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		ret += _targets_target(prefs, config, fp, targets);
		if(c == '\0')
			break;
		targets[i] = c;
		targets+=i+1;
		i = 0;
	}
	return ret;
}

static int _targets_all(Prefs * prefs, Config * config, FILE * fp)
{
	if(prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\nall:");
	if(config_get(config, "", "subdirs") != NULL)
		fprintf(fp, "%s", " subdirs");
	if(config_get(config, "", "targets") != NULL)
		fprintf(fp, "%s", " $(TARGETS)");
	fprintf(fp, "%s", "\n");
	return 0;
}

static int _targets_subdirs(Prefs * prefs, Config * config, FILE * fp)
{
	String * subdirs;

	if(prefs->flags & PREFS_n)
		return 0;
	if((subdirs = config_get(config, "", "subdirs")) != NULL)
		fprintf(fp, "%s", "\nsubdirs:\n\t@for i in $(SUBDIRS); do"
				" (cd $$i && $(MAKE)) || exit; done\n");
	return 0;
}

static int _target_objs(Prefs * prefs, Config * config, FILE * fp,
		String * target);
static int _target_binary(Prefs * prefs, Config * config, FILE * fp,
		String * target);
static int _target_library(Prefs * prefs, Config * config, FILE * fp,
		String * target);
static int _targets_target(Prefs * prefs, Config * config, FILE * fp,
		String * target)
{
	String * type;
	TargetType tt;
	String * p;

	if((type = config_get(config, target, "type")) == NULL)
	{
		fprintf(stderr, "%s%s%s", "configure: ", target,
				": no type defined for target\n");
		return 1;
	}
	tt = enum_string(TT_LAST, sTargetType, type);
	switch(tt)
	{
		case TT_BINARY:
			return _target_binary(prefs, config, fp, target);
		case TT_LIBRARY:
			return _target_library(prefs, config, fp, target);
		case TT_OBJECT:
			if((p = config_get(config, target, "sources")) == NULL)
			{
				fprintf(stderr, "%s%s%s", "configure: ", target,
						" no sources for target\n");
				return 1;
			}
			if(prefs->flags & PREFS_n)
				return 0;
			fprintf(fp, "%s%s%s%s", target, ": ", p, "\n");
			/* FIXME */
			break;
		case TT_UNKNOWN:
			fprintf(stderr, "%s%s%s", "configure: ", target,
					": unknown type for target\n");
			return 1;
	}
	return 0;
}

static int _objs_source(Prefs * prefs, FILE * fp, String * source);
static int _target_objs(Prefs * prefs, Config * config, FILE * fp,
		String * target)
{
	int ret = 0;
	String * sources;
	int i;
	char c;

	if((sources = config_get(config, target, "sources")) == NULL)
	{
		fprintf(stderr, "%s%s%s", "configure: ", target,
				": no sources defined for target\n");
		return 1;
	}
	if(!(prefs->flags & PREFS_n))
		fprintf(fp, "%s%s%s", "\n", target, "_OBJS =");
	for(i = 0; ret == 0; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		ret = _objs_source(prefs, fp, sources);
		if(c == '\0')
			break;
		sources[i] = c;
		sources+=i+1;
		i = 0;
	}
	if(!(prefs->flags & PREFS_n))
		fputc('\n', fp);
	return ret;
}

static int _objs_source(Prefs * prefs, FILE * fp, String * source)
{
	int ret = 0;
	String * extension;
	int len;

	if((extension = _source_extension(source)) == NULL)
	{
		fprintf(stderr, "%s%s%s", "configure: ", source,
				": no extension for source\n");
		return 1;
	}
	len = string_length(source) - string_length(extension) - 1;
	source[len] = '\0';
	switch(enum_string(OT_LAST, sObjectType, extension))
	{
		case OT_ASM_SOURCE:
		case OT_C_SOURCE:
			if(prefs->flags & PREFS_n)
				break;
			fprintf(fp, "%s%s%s", " ", source, ".o");
			break;
		case OT_UNKNOWN:
			ret = 1;
			fprintf(stderr, "%s%s%s", "configure: ", source,
					": unknown extension for source\n");
			break;
	}
	source[len] = '.';
	return ret;
}

static int _target_binary(Prefs * prefs, Config * config, FILE * fp,
		String * target)
{
	String * p;

	if(_target_objs(prefs, config, fp, target) != 0)
		return 1;
	if(prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s%s", target, "_CFLAGS = $(CFLAGSF) $(CFLAGS)");
	if((p = config_get(config, target, "cflags")) != NULL)
		fprintf(fp, " %s", p);
	fputc('\n', fp);
	fprintf(fp, "%s%s%s%s", target, ": $(", target, "_OBJS)\n");
	fprintf(fp, "%s", "\t$(CC) $(LDFLAGSF)");
	if((p = config_get(config, target, "ldflags_force")) != NULL)
		fprintf(fp, " %s", p);
	/* FIXME also find a way to add ldflags */
	fprintf(fp, "%s%s%s%s%s", " $(LDFLAGS) -o ", target, " $(", target,
			"_OBJS)\n");
	return 0;
}

static int _target_library(Prefs * prefs, Config * config, FILE * fp,
		String * target)
{
	String * p;

	if(_target_objs(prefs, config, fp, target) != 0)
		return 1;
	if(prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s%s", target, "_CFLAGS = $(CFLAGSF) $(CFLAGS)");
	if((p = config_get(config, target, "cflags")) != NULL)
		fprintf(fp, "%s%s", " ", p);
	fputc('\n', fp);
	fprintf(fp, "%s%s%s%s", target, ".a: $(", target, "_OBJS)\n");
	fprintf(fp, "%s%s%s%s%s", "\t$(AR) ", target, ".a $(", target,
			"_OBJS)\n");
	fprintf(fp, "%s%s%s", "\t$(RANLIB) ", target, ".a\n");
	fprintf(fp, "%s%s%s%s", target, ".so: $(", target, "_OBJS)\n");
	fprintf(fp, "%s%s%s%s%s", "\t$(LD) -o ", target, ".so $(", target,
			"_OBJS)\n");
	return 0;
}

static int _objects_target(Prefs * prefs, Config * config, FILE * fp,
		String * target);
static int _write_objects(Prefs * prefs, Config * config, FILE * fp)
{
	char * targets = config_get(config, "", "targets");
	char c;
	int i;
	int ret = 0;

	if(targets == NULL)
		return 0;
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		ret += _objects_target(prefs, config, fp, targets);
		if(c == '\0')
			break;
		targets[i] = c;
		targets+=i+1;
		i = 0;
	}
	return ret;
}

static int _target_source(Prefs * prefs, Config * config, FILE * fp,
		String * target, String * source);
static int _objects_target(Prefs * prefs, Config * config, FILE * fp,
		String * target)
{
	String * sources;
	int i;
	char c;

	if((sources = config_get(config, target, "sources")) == NULL)
		return 0;
	for(i = 0;; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		_target_source(prefs, config, fp, target, sources);
		if(c == '\0')
			break;
		sources[i] = c;
		sources+=i+1;
		i = 0;
	}
	return 0;
}

static void _source_c_depends(Config * config, FILE * fp, String * source);
static int _target_source(Prefs * prefs, Config * config, FILE * fp,
		String * target, String * source)
{
	int ret = 0;
	String * extension;
	ObjectType ot;
	int len;

	if((extension = _source_extension(source)) == NULL)
		return 1;
	len = string_length(source) - string_length(extension) - 1;
	source[len] = '\0';
	switch((ot = enum_string(OT_LAST, sObjectType, extension)))
	{
		case OT_ASM_SOURCE:
		case OT_C_SOURCE:
			if(prefs->flags & PREFS_n)
				break;
			fprintf(fp, "%s%s%s%s%s%s", "\n", source, ".o: ",
					source, ".", sObjectType[ot]);
			source[len] = '.'; /* FIXME ugly */
			_source_c_depends(config, fp, source);
			source[len] = '\0';
			fputc('\n', fp);
			fprintf(fp, "%s%s%s%s%s%s", "\t$(CC) $(", target,
					"_CFLAGS) -c ", source, ".",
					sObjectType[ot]);
			if(string_find(source, "/"))
				fprintf(fp, "%s%s%s", " -o ", source, ".o");
			fputc('\n', fp);
			break;
		case OT_UNKNOWN:
			ret = 1;
			break;
	}
	source[len] = '.';
	return ret;
}

static void _source_c_depends(Config * config, FILE * fp, String * source)
{
	String * depends;
	int i;
	char c;

	if((depends = config_get(config, source, "depends")) == NULL)
		return;
	for(i = 0;; i++)
	{
		if(depends[i] != ',' && depends[i] != '\0')
			continue;
		c = depends[i];
		depends[i] = '\0';
		fprintf(fp, " %s", depends);
		if(c == '\0')
			break;
		depends[i] = c;
		depends+=i+1;
		i = 0;
	}
}

static int _clean_targets(Config * config, FILE * fp);
static int _write_clean(Prefs * prefs, Config * config, FILE * fp)
{
	if(prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\nclean:\n");
	if(config_get(config, "", "subdirs") != NULL)
		fprintf(fp, "%s", "\t@for i in $(SUBDIRS); do"
				" (cd $$i && $(MAKE) clean) || exit; done\n");
	return _clean_targets(config, fp);
}

static int _clean_targets(Config * config, FILE * fp)
{
	String * targets;
	int i;
	char c;

	if((targets = config_get(config, "", "targets")) == NULL)
		return 0;
	fprintf(fp, "%s", "\t$(RM)");
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		fprintf(fp, "%s%s%s", " $(", targets, "_OBJS)");
		if(c == '\0')
			break;
		targets[i] = c;
		targets+=i+1;
		i = 0;
	}
	fputc('\n', fp);
	return 0;
}

static int _write_distclean(Prefs * prefs, Config * config, FILE * fp)
{
	String * subdirs;

	if(prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\ndistclean:");
	if((subdirs = config_get(config, "", "subdirs")) == NULL)
		fprintf(fp, "%s", " clean\n");
	else
	{
		fprintf(fp, "%s", "\n\t@for i in $(SUBDIRS); do (cd $$i"
				" && $(MAKE) distclean) || exit; done\n");
		_clean_targets(config, fp);
	}
	if(config_get(config, "", "targets") != NULL)
		fprintf(fp, "%s", "\t$(RM) $(TARGETS)\n");
	return 0;
}

static int _dist_subdir(Config * config, FILE * fp, Config * subdir);
static int _write_dist(Prefs * prefs, Config * config, FILE * fp,
		configArray * ca, int from, int to)
{
	String const * package;
	String const * version;
	Config * p;
	int i;

	if(prefs->flags & PREFS_n)
		return 0;
	if((package = config_get(config, "", "package")) == NULL
			|| (version = config_get(config, "", "version"))
			== NULL)
		return 0;
	fprintf(fp, "%s", "\ndist:\n"
			"\t$(RM) $(PACKAGE)-$(VERSION)\n"
			"\t$(LN) . $(PACKAGE)-$(VERSION)\n"
			"\t@$(TAR) $(PACKAGE)-$(VERSION).tar.gz \\\n");
	for(i = from+1; i < to; i++)
	{
		array_get_copy(ca, i, &p);
		_dist_subdir(config, fp, p);
	}
	if(from < to)
	{
		array_get_copy(ca, from, &p);
		_dist_subdir(config, fp, p);
	}
	else
		return 1;
	fprintf(fp, "%s", "\t$(RM) $(PACKAGE)-$(VERSION)\n");
	return 0;
}

static int _dist_subdir_dist(FILE * fp, String * path, String * dist);
static int _dist_subdir(Config * config, FILE * fp, Config * subdir)
{
	String * path;
	int len;
	String * targets;
	String * dist;
	int i;
	char c;

	path = config_get(config, "", "directory");
	len = string_length(path);
	path = config_get(subdir, "", "directory");
	path = &path[len];
	if(path[0] == '/')
		path++;
	if((targets = config_get(subdir, "", "targets")) != NULL)
		/* FIXME unique SOURCES */
		for(i = 0;; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			if((dist = config_get(subdir, targets, "sources"))
					!= NULL)
				_dist_subdir_dist(fp, path, dist);
			if(c == '\0')
				break;
			targets[i] = c;
			targets+=i+1;
			i = 0;
		}
	if((dist = config_get(subdir, "", "dist")) != NULL)
		_dist_subdir_dist(fp, path, dist);
	fprintf(fp, "%s%s%s%s", "\t\t$(PACKAGE)-$(VERSION)/", path,
			path[0] == '\0' ? "" : "/", PROJECT_CONF " \\\n");
	fprintf(fp, "%s%s%s%s%s", "\t\t$(PACKAGE)-$(VERSION)/", path,
			path[0] == '\0' ? "" : "/", MAKEFILE,
			path[0] == '\0' ? "\n" : " \\\n");
	return 0;
}

static int _dist_subdir_dist(FILE * fp, String * path, String * dist)
{
	int i;
	char c;

	for(i = 0;; i++)
	{
		if(dist[i] != ',' && dist[i] != '\0')
			continue;
		c = dist[i];
		dist[i] = '\0';
		fprintf(fp, "%s%s%s%s%s", "\t\t$(PACKAGE)-$(VERSION)/",
				path[0] == '\0' ? "" : path,
				path[0] == '\0' ? "" : "/", dist, " \\\n");
		if(c == '\0')
			break;
		dist[i] = c;
		dist+=i+1;
		i = 0;
	}
	return 0;
}

static int _install_target(Config * config, FILE * fp, String * target);
static int _write_install(Prefs * prefs, Config * config, FILE * fp)
{
	int ret = 0;
	String * subdirs;
	String * targets;
	int i;
	char c;

	if(prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\ninstall: all\n");
	if((subdirs = config_get(config, "", "subdirs")) != NULL)
		fprintf(fp, "%s", "\t@for i in $(SUBDIRS); do"
				" (cd $$i && $(MAKE) install) || exit; done\n");
	if((targets = config_get(config, "", "targets")) != NULL)
		for(i = 0; ret == 0; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			ret = _install_target(config, fp, targets);
			if(c == '\0')
				break;
			targets[i] = c;
			targets+=i+1;
			i = 0;
		}
	return ret;
}

static int _install_target(Config * config, FILE * fp, String * target)
{
	String * type;
	static Config * flag = NULL;
	static int done[TT_LAST];
	TargetType tt;

	if((type = config_get(config, target, "type")) == NULL)
		return 1;
	if(flag != config)
	{
		flag = config;
		memset(done, 0, sizeof(done));
	}
	switch((tt = enum_string(TT_LAST, sTargetType, type)))
	{
		case TT_BINARY:
			if(!done[tt])
				fprintf(fp, "%s", "\t$(MKDIR) $(DESTDIR)"
						"$(BINDIR)\n");
			fprintf(fp, "%s%s%s%s%s", "\t$(INSTALL) -m 0755 ",
					target, " $(DESTDIR)$(BINDIR)/",
					target, "\n");
			break;
		case TT_LIBRARY:
			if(!done[tt])
				fprintf(fp, "%s", "\t$(MKDIR) $(DESTDIR)"
						"$(LIBDIR)\n");
			fprintf(fp, "%s%s%s%s%s", "\t$(INSTALL) -m 0644 ",
					target, ".a $(DESTDIR)$(LIBDIR)/",
					target, ".a\n");
			fprintf(fp, "%s%s%s%s%s", "\t$(INSTALL) -m 0755 ",
					target, ".so $(DESTDIR)$(LIBDIR)/",
					target, ".so\n");
			break;
		case TT_OBJECT:
		case TT_UNKNOWN:
			break;
	}
	done[tt] = 1;
	return 0;
}

static int _uninstall_target(Config * config, FILE * fp, String * target);
static int _write_uninstall(Prefs * prefs, Config * config, FILE * fp)
{
	int ret = 0;
	String * subdirs;
	String * targets;
	int i;
	char c;

	if(prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\nuninstall:\n");
	if((subdirs = config_get(config, "", "subdirs")) != NULL)
		fprintf(fp, "%s", "\t@for i in $(SUBDIRS); do (cd $$i &&"
				" $(MAKE) uninstall) || exit; done\n");
	if((targets = config_get(config, "", "targets")) != NULL)
		for(i = 0; ret == 0; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			ret = _uninstall_target(config, fp, targets);
			if(c == '\0')
				break;
			targets[i] = c;
			targets+=i+1;
			i = 0;
		}
	return ret;
}

static int _uninstall_target(Config * config, FILE * fp, String * target)
{
	String * type;

	if((type = config_get(config, target, "type")) == NULL)
		return 1;
	switch(enum_string(TT_LAST, sTargetType, type))
	{
		case TT_BINARY:
			fprintf(fp, "%s%s%s", "\t$(RM) $(DESTDIR)$(BINDIR)/", target, "\n");
			break;
		case TT_LIBRARY:
			fprintf(fp, "%s%s%s", "\t$(RM) $(DESTDIR)$(LIBDIR)/", target, ".a\n");
			fprintf(fp, "%s%s%s", "\t$(RM) $(DESTDIR)$(LIBDIR)/", target, ".so\n");
			break;
		case TT_OBJECT:
		case TT_UNKNOWN:
			break;
	}
	return 0;
}
