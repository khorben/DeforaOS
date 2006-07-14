/* makefile.c */



#include <System.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "settings.h"
#include "configure.h"

ARRAY(Config *, config);


/* functions */
static int _makefile_write(Configure * configure, FILE * fp, configArray * ca,
	       	int from, int to);
int makefile(Configure * configure, String * directory, configArray * ca,
	       	int from, int to)
		
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
		ret |= _makefile_write(configure, fp, ca, from, to);
		if(fp != NULL)
			fclose(fp);
	}
	string_delete(makefile);
	return ret;
}

static int _write_variables(Configure * configure, FILE * fp);
static int _write_targets(Configure * configure, FILE * fp);
static int _write_objects(Configure * configure, FILE * fp);
static int _write_clean(Configure * configure, FILE * fp);
static int _write_distclean(Configure * configure, FILE * fp);
static int _write_dist(Configure * configure, FILE * fp, configArray * ca,
	       	int from, int to);
static int _write_install(Configure * configure, FILE * fp);
static int _write_uninstall(Configure * configure, FILE * fp);
static int _makefile_write(Configure * configure, FILE * fp, configArray * ca,
	       	int from, int to)
{
	Config * config = configure->config;

	if(_write_variables(configure, fp) != 0
			|| _write_targets(configure, fp) != 0
			|| _write_objects(configure, fp) != 0
			|| _write_clean(configure, fp) != 0
			|| _write_distclean(configure, fp) != 0
			|| _write_dist(configure, fp, ca, from, to) != 0
			|| _write_install(configure, fp) != 0
			|| _write_uninstall(configure, fp) != 0)
		return 1;
	if(!(configure->prefs->flags & PREFS_n))
		fprintf(fp, "%s%s%s%s%s", "\n.PHONY: all",
				config_get(config, "", "subdirs") != NULL
				? " subdirs" : "",
				" clean distclean",
				config_get(config, "", "package") != NULL
				&& config_get(config, "", "version") != NULL
				? " dist" : "",
				" install uninstall\n");
	return 0;
}

static int _variables_package(Configure * configure, FILE * fp,
		String const * directory);
static int _variables_print(Configure * configure, FILE * fp,
	       	char const * input, char const * output);
static int _variables_targets(Configure * configure, FILE * fp);
static int _variables_executables(Configure * configure, FILE * fp);
static int _write_variables(Configure * configure, FILE * fp)
{
	String const * directory = config_get(configure->config, "",
		       	"directory");
	int ret = 0;

	ret |= _variables_package(configure, fp, directory);
	ret |= _variables_print(configure, fp, "subdirs", "SUBDIRS");
	ret |= _variables_targets(configure, fp);
	ret |= _variables_executables(configure, fp);
	if(!(configure->prefs->flags & PREFS_n))
		fputc('\n', fp);
	return ret;
}

static int _variables_package(Configure * configure, FILE * fp,
		String const * directory)
{
	String * package;
	String * version;
	String * p;

	if((package = config_get(configure->config, "", "package")) == NULL)
		return 0;
	if(configure->prefs->flags & PREFS_v)
		printf("%s%s", "Package: ", package);
	if((version = config_get(configure->config, "", "version")) == NULL)
	{
		if(configure->prefs->flags & PREFS_v)
			fputc('\n', stdout);
		fprintf(stderr, "%s%s%s", "configure: ", directory,
				": \"package\" needs \"version\"\n");
		return 1;
	}
	if(configure->prefs->flags & PREFS_v)
		printf("%s%s%s", " ", version, "\n");
	if(fp != NULL)
		fprintf(fp, "%s%s%s%s%s", "PACKAGE\t= ", package,
				"\nVERSION\t= ", version, "\n");
	if((p = config_get(configure->config, "", "config")) != NULL)
		return settings(configure->prefs, configure->config, directory,
			       	package, version);
	return 0;
}

static int _variables_print(Configure * configure, FILE * fp,
		char const * input, char const * output)
{
	String * prints;
	unsigned long i;
	char c;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	if((prints = config_get(configure->config, "", input)) == NULL)
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

static int _variables_targets(Configure * configure, FILE * fp)
{
	String * prints;
	unsigned long i;
	char c;
	String * type;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	if((prints = config_get(configure->config, "", "targets")) == NULL)
		return 0;
	fprintf(fp, "%s%s", "TARGETS", "\t=");
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		if((type = config_get(configure->config, prints, "type"))
			       	== NULL)
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

static int _executables_variables(Configure * configure, FILE * fp,
	       	String * target);
static int _variables_executables(Configure * configure, FILE * fp)
{
	String * targets;
	int i;
	char c;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	if((targets = config_get(configure->config, "", "targets")) != NULL)
	{
		for(i = 0;; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			_executables_variables(configure, fp, targets);
			if(c == '\0')
				break;
			targets[i] = c;
			targets+=i+1;
			i = 0;
		}
		fprintf(fp, "%s", "RM\t= rm -f\n");
	}
	if(config_get(configure->config, "", "package"))
		fprintf(fp, "%s", "LN\t= ln -sf\nTAR\t= tar -czvf\n");
	if(targets != NULL)
	{
		fprintf(fp, "%s", "MKDIR\t= mkdir -p\n");
		fprintf(fp, "%s", "INSTALL\t= install\n");
	}
	return 0;
}

static void _variables_binary(Configure * configure, FILE * fp, char * done);
static void _variables_library(Configure * configure, FILE * fp, char * done);
static int _executables_variables(Configure * configure, FILE * fp,
	       	String * target)
{
	static Config * flag = NULL;
	String * type;
	char done[TT_LAST]; /* FIXME even better if'd be variable by variable */
	TargetType tt;

	if(flag != configure->config)
	{
		flag = configure->config;
		memset(done, 0, sizeof(done));
	}
	if((type = config_get(configure->config, target, "type")) == NULL)
		return 0;
	if(done[(tt = enum_string(TT_LAST, sTargetType, type))])
		return 0;
	switch(tt)
	{
		case TT_BINARY:
			_variables_binary(configure, fp, done);
			break;
		case TT_LIBRARY:
			_variables_library(configure, fp, done);
			break;
		case TT_OBJECT:
		case TT_UNKNOWN:
			break;
	}
	done[tt] = 1;
	return 0;
}

static void _targets_cflags(Configure * configure, FILE * fp);
static void _targets_cxxflags(Configure * configure, FILE * fp);
static void _binary_ldflags(Configure * configure, FILE * fp,
		String const * ldflags);
static void _variables_binary(Configure * configure, FILE * fp, char * done)
{
	String const * p;

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
		_targets_cflags(configure, fp);
		_targets_cxxflags(configure, fp);
	}
	if((p = config_get(configure->config, "", "ldflags_force")) != NULL)
	{
		fprintf(fp, "%s", "LDFLAGSF= ");
		_binary_ldflags(configure, fp, p);
	}
	if((p = config_get(configure->config, "", "ldflags")) != NULL)
	{
		fprintf(fp, "%s", "LDFLAGS\t= ");
		_binary_ldflags(configure, fp, p);
	}
}

static void _targets_cflags(Configure * configure, FILE * fp)
{
	String const * p;
	String const * q;

	if((p = config_get(configure->config, "", "cflags_force")) != NULL)
	{
		fprintf(fp, "%s%s", "CC\t= cc\nCFLAGSF\t= ", p);
		if(configure->os == HO_GNU_LINUX && string_find(p, "-ansi"))
			fprintf(fp, "%s", " -D _GNU_SOURCE"); /* FIXME undup */
		fputc('\n', fp);
	}
	if((q = config_get(configure->config, "", "cflags")) != NULL)
	{
		if(p == NULL)
			fprintf(fp, "%s", "CC\t= cc\n");
		fprintf(fp, "%s%s", "CFLAGS\t= ", q);
		if(configure->os == HO_GNU_LINUX && string_find(q, "-ansi"))
			fprintf(fp, "%s", " -D _GNU_SOURCE");
		fputc('\n', fp);
	}
}

static void _targets_cxxflags(Configure * configure, FILE * fp)
{
	String const * p;
	String const * q;

	if((p = config_get(configure->config, "", "cxxflags_force")) != NULL)
	{
		fprintf(fp, "%s%s", "CXX\t= c++\nCXXFLAGSF= ", p);
		if(configure->os == HO_GNU_LINUX && string_find(p, "-ansi"))
			fprintf(fp, "%s", " -D _GNU_SOURCE");
		fputc('\n', fp);
	}
	if((q = config_get(configure->config, "", "cxxflags")) != NULL)
	{
		if(p == NULL)
			fprintf(fp, "%s", "CXX\t= c++\n");
		fprintf(fp, "%s%s", "CXXFLAGS= ", q);
		if(configure->os == HO_GNU_LINUX && string_find(q, "-ansi"))
			fprintf(fp, "%s", " -D _GNU_SOURCE");
		fputc('\n', fp);
	}
}

static void _binary_ldflags(Configure * configure, FILE * fp,
		String const * ldflags)
{
	char * libs_gnu[] = { "socket", NULL };
	char * libs_bsd[] = { "crypt", "dl", "socket", NULL };
	char * libs_sunos[] = { "dl", NULL };
	char buf[10];
	char ** libs;
	String * p;
	String * q;
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
	fprintf(fp, "%s\n", p);
	free(p);
}

static void _variables_library(Configure * configure, FILE * fp, char * done)
{
	if(!done[TT_LIBRARY])
	{
		fprintf(fp, "%s%s\n", "PREFIX\t= ", configure->prefs->prefix);
		fprintf(fp, "%s%s\n", "DESTDIR\t= ", configure->prefs->destdir);
	}
	if(configure->prefs->libdir[0] == '/')
		fprintf(fp, "%s%s\n", "LIBDIR\t= ", configure->prefs->libdir);
	else
		fprintf(fp, "%s%s\n", "LIBDIR\t= $(PREFIX)/",
				configure->prefs->libdir);
	if(!done[TT_BINARY])
	{
		_targets_cflags(configure, fp);
		_targets_cxxflags(configure, fp);
	}
	fprintf(fp, "%s", "AR\t= ar -rc\n");
	fprintf(fp, "%s", "RANLIB\t= ranlib\n");
	fprintf(fp, "%s", "LD\t= ld -shared\n");
}

static int _targets_all(Configure * configure, FILE * fp);
static int _targets_subdirs(Configure * configure, FILE * fp);
static int _targets_target(Configure * configure, FILE * fp, String * target);
static int _write_targets(Configure * configure, FILE * fp)
{
	char * targets = config_get(configure->config, "", "targets");
	char c;
	int i;
	int ret = 0;

	if(_targets_all(configure, fp) != 0
			|| _targets_subdirs(configure, fp) != 0)
		return 1;
	if(targets == NULL)
		return 0;
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		ret += _targets_target(configure, fp, targets);
		if(c == '\0')
			break;
		targets[i] = c;
		targets+=i+1;
		i = 0;
	}
	return ret;
}

static int _targets_all(Configure * configure, FILE * fp)
{
	if(configure->prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\nall:");
	if(config_get(configure->config, "", "subdirs") != NULL)
		fprintf(fp, "%s", " subdirs");
	if(config_get(configure->config, "", "targets") != NULL)
		fprintf(fp, "%s", " $(TARGETS)");
	fprintf(fp, "%s", "\n");
	return 0;
}

static int _targets_subdirs(Configure * configure, FILE * fp)
{
	String * subdirs;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	if((subdirs = config_get(configure->config, "", "subdirs")) != NULL)
		fprintf(fp, "%s", "\nsubdirs:\n\t@for i in $(SUBDIRS); do"
				" (cd $$i && $(MAKE)) || exit; done\n");
	return 0;
}

static int _target_objs(Configure * configure, FILE * fp, String * target);
static int _target_binary(Configure * configure, FILE * fp, String * target);
static int _target_library(Configure * configure, FILE * fp, String * target);
static int _targets_target(Configure * configure, FILE * fp, String * target)
{
	String * type;
	TargetType tt;
	String * p;

	if((type = config_get(configure->config, target, "type")) == NULL)
	{
		fprintf(stderr, "%s%s%s", "configure: ", target,
				": no type defined for target\n");
		return 1;
	}
	tt = enum_string(TT_LAST, sTargetType, type);
	switch(tt)
	{
		case TT_BINARY:
			return _target_binary(configure, fp, target);
		case TT_LIBRARY:
			return _target_library(configure, fp, target);
		case TT_OBJECT:
			if((p = config_get(configure->config, target,
						       	"sources")) == NULL)
			{
				fprintf(stderr, "%s%s%s", "configure: ", target,
						" no sources for target\n");
				return 1;
			}
			if(configure->prefs->flags & PREFS_n)
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
static int _target_objs(Configure * configure, FILE * fp, String * target)
{
	int ret = 0;
	String * sources;
	int i;
	char c;

	if((sources = config_get(configure->config, target, "sources")) == NULL)
	{
		fprintf(stderr, "%s%s%s", "configure: ", target,
				": no sources defined for target\n");
		return 1;
	}
	if(!(configure->prefs->flags & PREFS_n))
		fprintf(fp, "%s%s%s", "\n", target, "_OBJS =");
	for(i = 0; ret == 0; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		ret = _objs_source(configure->prefs, fp, sources);
		if(c == '\0')
			break;
		sources[i] = c;
		sources+=i+1;
		i = 0;
	}
	if(!(configure->prefs->flags & PREFS_n))
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
		case OT_CXX_SOURCE:
		case OT_CPP_SOURCE:
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

static void _target_flags(Configure * configure, FILE * fp, String * target);
static int _target_binary(Configure * configure, FILE * fp, String * target)
{
	String * p;

	if(_target_objs(configure, fp, target) != 0)
		return 1;
	if(configure->prefs->flags & PREFS_n)
		return 0;
	_target_flags(configure, fp, target);
	fprintf(fp, "%s%s%s%s", target, ": $(", target, "_OBJS)\n");
	fprintf(fp, "%s", "\t$(CC) $(LDFLAGSF)");
	if((p = config_get(configure->config, target, "ldflags")) != NULL)
		fprintf(fp, " %s", p);
	fprintf(fp, "%s%s%s%s%s", " $(LDFLAGS) -o ", target, " $(", target,
			"_OBJS)\n");
	return 0;
}

static void _flags_c(Configure * configure, FILE * fp, String * target);
static void _flags_cxx(Configure * configure, FILE * fp, String * target);
static void _target_flags(Configure * configure, FILE * fp, String * target)
{
	char done[OT_LAST+1];
	String * sources;
	String * extension;
	ObjectType type;
	char c;
	unsigned int i;

	memset(done, 0, sizeof(done));
	if((sources = config_get(configure->config, target, "sources")) == NULL)
		return;
	for(i = 0;; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		extension = _source_extension(sources);
		if(extension == NULL)
		{
			sources[i] = c;
			continue;
		}
		type = enum_string(OT_LAST, sObjectType, extension);
		sources[i] = c;
		if(!done[type])
			switch(type)
			{
				case OT_ASM_SOURCE:
					break;
				case OT_C_SOURCE:
					_flags_c(configure, fp, target);
					break;
				case OT_CXX_SOURCE:
				case OT_CPP_SOURCE:
					done[OT_CXX_SOURCE] = 1;
					done[OT_CPP_SOURCE] = 1;
					_flags_cxx(configure, fp, target);
					break;
				case OT_UNKNOWN:
					break;
			}
		done[type] = 1;
		if(c == '\0')
			break;
		sources+=i+1;
		i = 0;
	}
}

static void _flags_c(Configure * configure, FILE * fp, String * target)
{
	String const * p;

	fprintf(fp, "%s%s", target, "_CFLAGS = $(CFLAGSF) $(CFLAGS)");
	if((p = config_get(configure->config, target, "cflags")) != NULL)
	{
		fprintf(fp, " %s", p);
		if(configure->os == HO_GNU_LINUX && string_find(p, "-ansi"))
			fprintf(fp, "%s", " -D _GNU_SOURCE");
	}
	fputc('\n', fp);
}

static void _flags_cxx(Configure * configure, FILE * fp, String * target)
{
	String const * p;

	fprintf(fp, "%s%s", target, "_CXXFLAGS = $(CXXFLAGSF) $(CXXFLAGS)");
	if((p = config_get(configure->config, target, "cxxflags")) != NULL)
		fprintf(fp, " %s", p);
	fputc('\n', fp);
}

static int _target_library(Configure * configure, FILE * fp, String * target)
{
	if(_target_objs(configure, fp, target) != 0)
		return 1;
	if(configure->prefs->flags & PREFS_n)
		return 0;
	_target_flags(configure, fp, target);
	fprintf(fp, "%s%s%s%s", target, ".a: $(", target, "_OBJS)\n");
	fprintf(fp, "%s%s%s%s%s", "\t$(AR) ", target, ".a $(", target,
			"_OBJS)\n");
	fprintf(fp, "%s%s%s", "\t$(RANLIB) ", target, ".a\n");
	fprintf(fp, "%s%s%s%s", target, ".so: $(", target, "_OBJS)\n");
	fprintf(fp, "%s%s%s%s%s", "\t$(LD) -o ", target, ".so $(", target,
			"_OBJS)\n");
	return 0;
}

static int _objects_target(Configure * configure, FILE * fp, String * target);
static int _write_objects(Configure * configure, FILE * fp)
{
	char * targets = config_get(configure->config, "", "targets");
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
		ret += _objects_target(configure, fp, targets);
		if(c == '\0')
			break;
		targets[i] = c;
		targets+=i+1;
		i = 0;
	}
	return ret;
}

static int _target_source(Configure * configure, FILE * fp, String * target,
		String * source);
static int _objects_target(Configure * configure, FILE * fp, String * target)
{
	String * sources;
	int i;
	char c;

	if((sources = config_get(configure->config, target, "sources")) == NULL)
		return 0;
	for(i = 0;; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		_target_source(configure, fp, target, sources);
		if(c == '\0')
			break;
		sources[i] = c;
		sources+=i+1;
		i = 0;
	}
	return 0;
}

static void _source_c_depends(Config * config, FILE * fp, String * source);
static int _target_source(Configure * configure, FILE * fp, String * target,
		String * source)
{
	int ret = 0;
	String * extension;
	ObjectType ot;
	int len;
	String const * p;

	if((extension = _source_extension(source)) == NULL)
		return 1;
	len = string_length(source) - string_length(extension) - 1;
	source[len] = '\0';
	switch((ot = enum_string(OT_LAST, sObjectType, extension)))
	{
		case OT_ASM_SOURCE:
		case OT_C_SOURCE:
			if(configure->prefs->flags & PREFS_n)
				break;
			fprintf(fp, "%s%s%s%s%s%s", "\n", source, ".o: ",
					source, ".", sObjectType[ot]);
			source[len] = '.'; /* FIXME ugly */
			_source_c_depends(configure->config, fp, source);
			p = config_get(configure->config, source, "cflags");
			source[len] = '\0';
			fprintf(fp, "%s%s%s", "\n\t$(CC) $(", target,
				       	"_CFLAGS)");
			if(p != NULL)
			{
				fprintf(fp, " %s", p);
				if(configure->os == HO_GNU_LINUX
					       	&& string_find(p, "-ansi"))
					fprintf(fp, "%s", " -D _GNU_SOURCE");
			}
			fprintf(fp, "%s%s%s%s", " -c ", source, ".",
					sObjectType[ot]);
			if(string_find(source, "/"))
				fprintf(fp, "%s%s%s", " -o ", source, ".o");
			fputc('\n', fp);
			break;
		case OT_CXX_SOURCE:
		case OT_CPP_SOURCE:
			if(configure->prefs->flags & PREFS_n)
				break;
			fprintf(fp, "%s%s%s%s%s%s", "\n", source, ".o: ",
					source, ".", sObjectType[ot]);
			source[len] = '.'; /* FIXME ugly */
			_source_c_depends(configure->config, fp, source);
			p = config_get(configure->config, source, "cxxflags");
			source[len] = '\0';
			fprintf(fp, "%s%s%s", "\n\t$(CXX) $(", target,
					"_CXXFLAGS)");
			if(p != NULL)
				fprintf(fp, " %s", p);
			fprintf(fp, "%s%s%s%s", " -c ", source, ".",
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
static int _write_clean(Configure * configure, FILE * fp)
{
	if(configure->prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\nclean:\n");
	if(config_get(configure->config, "", "subdirs") != NULL)
		fprintf(fp, "%s", "\t@for i in $(SUBDIRS); do"
				" (cd $$i && $(MAKE) clean) || exit; done\n");
	return _clean_targets(configure->config, fp);
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

static int _write_distclean(Configure * configure, FILE * fp)
{
	String * subdirs;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\ndistclean:");
	if((subdirs = config_get(configure->config, "", "subdirs")) == NULL)
		fprintf(fp, "%s", " clean\n");
	else
	{
		fprintf(fp, "%s", "\n\t@for i in $(SUBDIRS); do (cd $$i"
				" && $(MAKE) distclean) || exit; done\n");
		_clean_targets(configure->config, fp);
	}
	if(config_get(configure->config, "", "targets") != NULL)
		fprintf(fp, "%s", "\t$(RM) $(TARGETS)\n");
	return 0;
}

static int _dist_subdir(Config * config, FILE * fp, Config * subdir);
static int _write_dist(Configure * configure, FILE * fp, configArray * ca,
	       	int from, int to)
{
	String const * package;
	String const * version;
	Config * p;
	int i;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	if((package = config_get(configure->config, "", "package")) == NULL
			|| (version = config_get(configure->config, "",
				       	"version")) == NULL)
		return 0;
	fprintf(fp, "%s", "\ndist:\n"
			"\t$(RM) -r $(PACKAGE)-$(VERSION)\n"
			"\t$(LN) . $(PACKAGE)-$(VERSION)\n"
			"\t@$(TAR) $(PACKAGE)-$(VERSION).tar.gz \\\n");
	for(i = from+1; i < to; i++)
	{
		array_get_copy(ca, i, &p);
		_dist_subdir(configure->config, fp, p);
	}
	if(from < to)
	{
		array_get_copy(ca, from, &p);
		_dist_subdir(configure->config, fp, p);
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
	fprintf(fp, "%s%s%s%s%s", "\t\t$(PACKAGE)-$(VERSION)/", path,
			path[0] == '\0' ? "" : "/", PROJECT_CONF,
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
static int _write_install(Configure * configure, FILE * fp)
{
	int ret = 0;
	String * subdirs;
	String * targets;
	int i;
	char c;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\ninstall: all\n");
	if((subdirs = config_get(configure->config, "", "subdirs")) != NULL)
		fprintf(fp, "%s", "\t@for i in $(SUBDIRS); do"
				" (cd $$i && $(MAKE) install) || exit; done\n");
	if((targets = config_get(configure->config, "", "targets")) != NULL)
		for(i = 0; ret == 0; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			ret = _install_target(configure->config, fp, targets);
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
static int _write_uninstall(Configure * configure, FILE * fp)
{
	int ret = 0;
	String * subdirs;
	String * targets;
	int i;
	char c;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "%s", "\nuninstall:\n");
	if((subdirs = config_get(configure->config, "", "subdirs")) != NULL)
		fprintf(fp, "%s", "\t@for i in $(SUBDIRS); do (cd $$i &&"
				" $(MAKE) uninstall) || exit; done\n");
	if((targets = config_get(configure->config, "", "targets")) != NULL)
		for(i = 0; ret == 0; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			ret = _uninstall_target(configure->config, fp, targets);
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
			fprintf(fp, "%s%s%s", "\t$(RM) $(DESTDIR)$(BINDIR)/",
					target, "\n");
			break;
		case TT_LIBRARY:
			fprintf(fp, "%s%s%s", "\t$(RM) $(DESTDIR)$(LIBDIR)/",
					target, ".a\n");
			fprintf(fp, "%s%s%s", "\t$(RM) $(DESTDIR)$(LIBDIR)/",
					target, ".so\n");
			break;
		case TT_OBJECT:
		case TT_UNKNOWN:
			break;
	}
	return 0;
}
