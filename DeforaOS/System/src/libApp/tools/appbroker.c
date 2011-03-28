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
typedef struct _AppBrokerData
{
	char const * prefix;
	char const * outfile;
	FILE * fp;
} AppBrokerData;


/* functions */
static void _appbroker_head(AppBrokerData * data);
static int _appbroker_foreach(char const * key, Hash * value,
		AppBrokerData * data);
static int _appbroker_foreach_arg(AppBrokerData * data, char const * sep,
		char const * arg);
static void _appbroker_tail(AppBrokerData * data);

static int _appbroker(char const * outfile, char const * filename)
{
	Config * config;
	AppBrokerData data;

	if((config = config_new()) == NULL)
		return error_print(APPBROKER_PROGNAME);
	if(config_load(config, filename) != 0)
	{
		config_delete(config);
		return error_print(APPBROKER_PROGNAME);
	}
	data.prefix = config_get(config, NULL, "service");
	if((data.outfile = outfile) == NULL)
		data.fp = stdout;
	else if((data.fp = fopen(outfile, "w")) == NULL)
	{
		config_delete(config);
		return error_set_print(APPBROKER_PROGNAME, 1, "%s: %s", outfile,
				strerror(errno));
	}
	_appbroker_head(&data);
	fputs("\n\n/* functions */\n", data.fp);
	hash_foreach(config, (HashForeach)_appbroker_foreach, &data);
	_appbroker_tail(&data);
	if(outfile != NULL)
		fclose(data.fp);
	config_delete(config);
	return 0;
}

static void _appbroker_head(AppBrokerData * data)
{
	fputs("/* $""Id$ */\n\n\n\n", data->fp);
	if(data->prefix != NULL)
		fprintf(data->fp, "%s%s%s%s%s%s", "#ifndef ", data->prefix,
				"_H\n",	"# define ", data->prefix, "_H\n");
	fputs("\n# include <stdint.h>\n", data->fp);
	fputs("# include <System.h>\n\n", data->fp);
	fputs("\n/* types */\n", data->fp);
	fputs("typedef Buffer * BUFFER;\n", data->fp);
	fputs("typedef double * DOUBLE;\n", data->fp);
	fputs("typedef float * FLOAT;\n", data->fp);
	fputs("typedef int16_t INT16;\n", data->fp);
	fputs("typedef int32_t INT32;\n", data->fp);
	fputs("typedef uint16_t UINT16;\n", data->fp);
	fputs("typedef uint32_t UINT32;\n", data->fp);
	fputs("typedef String const * STRING;\n", data->fp);
	fputs("typedef void VOID;\n", data->fp);
	fputs("\ntypedef BUFFER BUFFER_IN;\n", data->fp);
	fputs("\ntypedef DOUBLE DOUBLE_IN;\n", data->fp);
	fputs("\ntypedef FLOAT FLOAT_IN;\n", data->fp);
	fputs("typedef INT32 INT32_IN;\n", data->fp);
	fputs("typedef UINT32 UINT32_IN;\n", data->fp);
	fputs("typedef STRING STRING_IN;\n", data->fp);
	fputs("\ntypedef Buffer * BUFFER_OUT;\n", data->fp);
	fputs("typedef int32_t * INT32_OUT;\n", data->fp);
	fputs("typedef uint32_t * UINT32_OUT;\n", data->fp);
	fputs("typedef String ** STRING_OUT;\n", data->fp);
	fputs("\ntypedef Buffer * BUFFER_INOUT;\n", data->fp);
	fputs("typedef int32_t * INT32_INOUT;\n", data->fp);
	fputs("typedef uint32_t * UINT32_INOUT;\n", data->fp);
	fputs("typedef String ** STRING_INOUT;\n", data->fp);
}

static int _appbroker_foreach(char const * key, Hash * value,
		AppBrokerData * data)
{
	int i;
	char buf[8];
	char const * p;
	char const * sep = "";

	if(key == NULL || key[0] == '\0')
		return 0;
	if((p = hash_get(value, "ret")) == NULL)
		p = "void";
	fprintf(data->fp, "%s%s%s%s%s%s", p, " ", data->prefix, "_", key, "(");
	for(i = 0; i < APPSERVER_MAX_ARGUMENTS; i++)
	{
		snprintf(buf, sizeof(buf), "arg%d", i + 1);
		if((p = hash_get(value, buf)) == NULL)
			break;
		if(_appbroker_foreach_arg(data, sep, p) != 0)
			return 1;
		sep = ", ";
	}
	fprintf(data->fp, "%s", ");\n");
	return 0;
}

static int _appbroker_foreach_arg(AppBrokerData * data, char const * sep,
		char const * arg)
{
	char * p;
	size_t size;

	if((p = strchr(arg, ',')) == NULL)
	{
		fprintf(data->fp, "%s%s", sep, arg);
		return 0;
	}
	fputs(sep, data->fp);
	size = p - arg;
	if(fwrite(arg, sizeof(*arg), size, data->fp) != size)
		return 1;
	if(*(++p) != '\0')
		fprintf(data->fp, " %s", p);
	return 0;
}

static void _appbroker_tail(AppBrokerData * data)
{
	if(data->prefix != NULL)
		fprintf(data->fp, "%s%s%s", "\n#endif /* !", data->prefix,
				"_H */\n");
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
