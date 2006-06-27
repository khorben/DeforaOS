/* settings.c */



#include <System.h>
#include <stdio.h>
#include <string.h>
#include "configure.h"
#include "settings.h"


/* types */
typedef enum _SETTINGS_TYPE
{
	ST_H = 0,
	ST_SH
} SettingsType;
#define ST_LAST ST_SH
String * sSettingsType[ST_LAST+1] =
{
	"h", "sh"
};


/* functions */
/* settings */
static int _settings_do(Prefs * prefs, String const * directory,
		String const * package, String const * version,
		String const * extension);
int settings(Prefs * prefs, Config * config, String const * directory,
		String const * package, String const * version)
{
	int ret = 0;
	String * p;
	unsigned long i;
	char c;

	if((p = config_get(config, "", "config")) == NULL)
		return 0;
	for(i = 0;; i++)
	{
		if(p[i] != ',' && p[i] != '\0')
			continue;
		c = p[i];
		p[i] = '\0';
		ret |= _settings_do(prefs, directory, package, version, p);
		if(c == '\0')
			break;
		p[i] = c;
		p+=i+1;
		i = 0;
	}
	return ret;
}

static int _do_h(FILE * fp, String const * package, String const * version);
static int _do_sh(FILE * fp, String const * package, String const * version);
static int _settings_do(Prefs * prefs, String const * directory,
		String const * package, String const * version,
		String const * extension)
{
	int ret = 0;
	int i;
	String * filename;
	FILE * fp;

	for(i = 0; i <= ST_LAST; i++)
		if(strcmp(extension, sSettingsType[i]) == 0)
			break;
	if(i > ST_LAST)
	{
		fprintf(stderr, "%s%s%s", "configure: ", extension,
				": Unknown settings type\n");
		return 1;
	}
	if(prefs->flags & PREFS_n)
		return 0;
	if((filename = string_new(directory)) == NULL)
		return 1;
	if(string_append(&filename, "/config.") != 0
			|| string_append(&filename, extension) != 0)
	{
		string_delete(filename);
		return 1;
	}
	if((fp = fopen(filename, "w")) == NULL)
		ret |= configure_error(filename, 1);
	else
	{
		if(prefs->flags & PREFS_v)
			printf("%s%s%s%s\n", "Creating config.", extension,
					" in ", directory);
		switch(i)
		{
			case ST_H:
				ret |= _do_h(fp, package, version);
				break;
			case ST_SH:
				ret |= _do_sh(fp, package, version);
				break;
			default:
				break;
		}
		fclose(fp);
	}
	string_delete(filename);
	return ret;
}

static int _do_h(FILE * fp, String const * package, String const * version)
{
	fprintf(fp, "%s%s%s%s%s%s", "#define PACKAGE \"", package, "\"\n",
			"#define VERSION \"", version, "\"\n");
	return 0;
}

static int _do_sh(FILE * fp, String const * package, String const * version)
{
	fprintf(fp, "%s%s%s%s%s%s", "PACKAGE=\"", package, "\"\n",
			"VERSION=\"", version, "\"\n");
	return 0;
}
