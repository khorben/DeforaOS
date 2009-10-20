/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
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



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


/* types */
typedef int Prefs;
#define RM_PREFS_f 0x1
#define RM_PREFS_i 0x2
#define RM_PREFS_R 0x4


/* rm */
static int _rm_error(char const * message, int ret);

static int _rm_do(Prefs * prefs, char * file);
static int _rm(Prefs * prefs, int argc, char * argv[])
{
	int ret = 0;
	int i;

	for(i = 0; i < argc; i++)
		ret |= _rm_do(prefs, argv[i]);
	return ret;
}

static int _rm_error(char const * message, int ret)
{
	fputs("rm: ", stderr);
	perror(message);
	return ret;
}

static int _rm_confirm(char const * message, char const * type)
{
	int c;
	int tmp;

	fprintf(stderr, "%s%s%s%s%s", "rm: ", message, ": Remove ", type, "? ");
	if((c = fgetc(stdin)) == EOF)
		return 0;
	while(c != '\n' && (tmp = fgetc(stdin)) != EOF && tmp != '\n');
	return c == 'y';
}

static int _rm_do_recursive(Prefs * prefs, char * file);
static int _rm_do(Prefs * prefs, char * file)
{
	struct stat st;

	if(lstat(file, &st) != 0 && errno == ENOENT)
	{
		if(!(*prefs & RM_PREFS_f))
			return _rm_error(file, 1);
		return 0;
	}
	if(S_ISDIR(st.st_mode))
	{
		if(!(*prefs & RM_PREFS_R))
		{
			errno = EISDIR;
			return _rm_error(file, *prefs & RM_PREFS_f ? 0 : 1);
		}
		return _rm_do_recursive(prefs, file);
	}
	/* FIXME ask also if permissions do not allow file removal */
	if(*prefs & RM_PREFS_i && !_rm_confirm(file, "file"))
		return 0;
	if(unlink(file) != 0)
		return _rm_error(file, 1);
	return 0;
}

static int _rm_do_recursive(Prefs * prefs, char * filename)
{
	int ret = 0;
	DIR * dir;
	struct dirent * de;
	size_t len = strlen(filename) + 2;
	char * path;
	char * p;

	if((dir = opendir(filename)) == NULL)
		return _rm_error(filename, 1);
	if((path = malloc(len)) == NULL)
	{
		closedir(dir);
		return _rm_error(filename, 1);
	}
	sprintf(path, "%s/", filename);
	while((de = readdir(dir)) != NULL)
	{
		if(de->d_name[0] == '.' && (de->d_name[1] == '\0'
					|| (de->d_name[1] == '.'
						&& de->d_name[2] == '\0')))
			continue;
		if((p = realloc(path, len + strlen(de->d_name))) == NULL)
			break;
		path = p;
		strcpy(&path[len - 1], de->d_name);
		ret |= _rm_do(prefs, path);
	}
	free(path);
	closedir(dir);
	if(de != NULL)
		return _rm_error(filename, 1);
	if(*prefs & RM_PREFS_i && !_rm_confirm(filename, "directory"))
		return ret;
	if(rmdir(filename) != 0)
		return _rm_error(filename, 1);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: rm [-fiRr] file...\n\
  -f    Do not prompt for confirmation or output error messages\n\
  -i    Prompt for confirmation\n\
  -R    Remove file hierarchies\n\
  -r    Equivalent to -R\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs = 0;
	int o;

	while((o = getopt(argc, argv, "fiRr")) != -1)
		switch(o)
		{
			case 'f':
				prefs -= prefs & RM_PREFS_i;
				prefs |= RM_PREFS_f;
				break;
			case 'i':
				prefs -= prefs & RM_PREFS_f;
				prefs |= RM_PREFS_i;
				break;
			case 'R':
			case 'r':
				prefs |= RM_PREFS_R;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	return _rm(&prefs, argc-optind, &argv[optind]) == 0 ? 0 : 2;
}
