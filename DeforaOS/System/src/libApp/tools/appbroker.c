/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libApp */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <System.h>
#include "System/App.h"

#define APPBROKER_PROGNAME "AppBroker"


/* AppBroker */
/* private */
/* types */
typedef struct _AppBroker
{
	Config * config;
	char const * prefix;
	char const * outfile;
	FILE * fp;
} AppBroker;


/* functions */
static void _appbroker_calls(AppBroker * appbroker);
static void _appbroker_constants(AppBroker * appbroker);
static int _appbroker_foreach_call(char const * key, Hash * value, void * data);
static int _appbroker_foreach_call_arg(AppBroker * appbroker, char const * sep,
		char const * arg);
static int _appbroker_foreach_constant(char const * key, char const * value,
		void * data);
static void _appbroker_head(AppBroker * appbroker);
static void _appbroker_tail(AppBroker * appbroker);

static int _appbroker(char const * outfile, char const * filename)
{
	AppBroker appbroker;

	if((appbroker.config = config_new()) == NULL)
		return error_print(APPBROKER_PROGNAME);
	if(config_load(appbroker.config, filename) != 0)
	{
		config_delete(appbroker.config);
		return error_print(APPBROKER_PROGNAME);
	}
	appbroker.prefix = config_get(appbroker.config, NULL, "service");
	if((appbroker.outfile = outfile) == NULL)
		appbroker.fp = stdout;
	else if((appbroker.fp = fopen(outfile, "w")) == NULL)
	{
		config_delete(appbroker.config);
		return error_set_print(APPBROKER_PROGNAME, 1, "%s: %s", outfile,
				strerror(errno));
	}
	_appbroker_head(&appbroker);
	_appbroker_constants(&appbroker);
	_appbroker_calls(&appbroker);
	_appbroker_tail(&appbroker);
	if(outfile != NULL)
		fclose(appbroker.fp);
	config_delete(appbroker.config);
	return 0;
}

static void _appbroker_calls(AppBroker * appbroker)
{
	fputs("\n\n/* calls */\n", appbroker->fp);
	hash_foreach(appbroker->config,
			(HashForeach)_appbroker_foreach_call, appbroker);
}

static void _appbroker_constants(AppBroker * appbroker)
{
	Hash * hash;

	if((hash = hash_get(appbroker->config, "constants")) == NULL)
		return;
	fputs("\n\n/* constants */\n", appbroker->fp);
	hash_foreach(hash, (HashForeach)_appbroker_foreach_constant, appbroker);
}

static int _appbroker_foreach_call(char const * key, Hash * value, void * data)
{
	AppBroker * appbroker = data;
	int i;
	char buf[8];
	char const * p;
	char const * sep = "";

	if(key == NULL || key[0] == '\0')
		return 0;
	if(strncmp(key, "call::", 6) != 0)
		return 0;
	key += 6;
	if((p = hash_get(value, "ret")) == NULL)
		p = "void";
	fprintf(appbroker->fp, "%s%s%s%s%s%s", p, " ", appbroker->prefix, "_",
			key, "(");
	for(i = 0; i < APPSERVER_MAX_ARGUMENTS; i++)
	{
		snprintf(buf, sizeof(buf), "arg%d", i + 1);
		if((p = hash_get(value, buf)) == NULL)
			break;
		if(_appbroker_foreach_call_arg(appbroker, sep, p) != 0)
			return 1;
		sep = ", ";
	}
	fprintf(appbroker->fp, "%s", ");\n");
	return 0;
}

static int _appbroker_foreach_call_arg(AppBroker * appbroker, char const * sep,
		char const * arg)
{
	char * p;
	size_t size;

	if((p = strchr(arg, ',')) == NULL)
	{
		fprintf(appbroker->fp, "%s%s", sep, arg);
		return 0;
	}
	fputs(sep, appbroker->fp);
	size = p - arg;
	if(fwrite(arg, sizeof(*arg), size, appbroker->fp) != size)
		return 1;
	if(*(++p) != '\0')
		fprintf(appbroker->fp, " %s", p);
	return 0;
}

static int _appbroker_foreach_constant(char const * key, char const * value,
		void * data)
{
	AppBroker * appbroker = data;

	fprintf(appbroker->fp, "# define %s_%s\t%s\n", appbroker->prefix,
			key, value);
	return 0;
}

static void _appbroker_head(AppBroker * appbroker)
{
	fputs("/* $""Id$ */\n\n\n\n", appbroker->fp);
	if(appbroker->prefix != NULL)
		fprintf(appbroker->fp, "%s%s%s%s%s%s", "#ifndef ",
				appbroker->prefix, "_H\n", "# define ",
				appbroker->prefix, "_H\n");
	fputs("\n# include <stdint.h>\n", appbroker->fp);
	fputs("# include <System.h>\n\n", appbroker->fp);
	fputs("\n/* types */\n", appbroker->fp);
	fputs("typedef Buffer * BUFFER;\n", appbroker->fp);
	fputs("typedef double * DOUBLE;\n", appbroker->fp);
	fputs("typedef float * FLOAT;\n", appbroker->fp);
	fputs("typedef int16_t INT16;\n", appbroker->fp);
	fputs("typedef int32_t INT32;\n", appbroker->fp);
	fputs("typedef uint16_t UINT16;\n", appbroker->fp);
	fputs("typedef uint32_t UINT32;\n", appbroker->fp);
	fputs("typedef String const * STRING;\n", appbroker->fp);
	fputs("typedef void VOID;\n", appbroker->fp);
	fputs("\ntypedef BUFFER BUFFER_IN;\n", appbroker->fp);
	fputs("\ntypedef DOUBLE DOUBLE_IN;\n", appbroker->fp);
	fputs("\ntypedef FLOAT FLOAT_IN;\n", appbroker->fp);
	fputs("typedef INT32 INT32_IN;\n", appbroker->fp);
	fputs("typedef UINT32 UINT32_IN;\n", appbroker->fp);
	fputs("typedef STRING STRING_IN;\n", appbroker->fp);
	fputs("\ntypedef Buffer * BUFFER_OUT;\n", appbroker->fp);
	fputs("typedef int32_t * INT32_OUT;\n", appbroker->fp);
	fputs("typedef uint32_t * UINT32_OUT;\n", appbroker->fp);
	fputs("typedef String ** STRING_OUT;\n", appbroker->fp);
	fputs("\ntypedef Buffer * BUFFER_INOUT;\n", appbroker->fp);
	fputs("typedef int32_t * INT32_INOUT;\n", appbroker->fp);
	fputs("typedef uint32_t * UINT32_INOUT;\n", appbroker->fp);
	fputs("typedef String ** STRING_INOUT;\n", appbroker->fp);
}

static void _appbroker_tail(AppBroker * appbroker)
{
	if(appbroker->prefix != NULL)
		fprintf(appbroker->fp, "%s%s%s", "\n#endif /* !",
				appbroker->prefix, "_H */\n");
}


/* usage */
static int _usage(void)
{
	fputs("Usage: " APPBROKER_PROGNAME " [-o outfile] filename\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * outfile = NULL;

	while((o = getopt(argc, argv, "o:")) != -1)
		switch(o)
		{
			case 'o':
				outfile = optarg;
				break;
			default:
				return _usage();
		}
	if(optind + 1 != argc)
		return _usage();
	return (_appbroker(outfile, argv[optind]) == 0) ? 0 : 2;
}
