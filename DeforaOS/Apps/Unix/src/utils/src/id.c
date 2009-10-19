/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
/* utils is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * utils is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with utils; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>


/* id */
static int _id_error(char const * message, int ret);
static int _id_G(char const * user, int flagn);
static int _id_g(char const * user, int flagn, int flagr);
static int _id_u(char const * user, int flagn, int flagr);
static int _id_all(char const * user);

static int _id(char const * user, int flag, int flagn, int flagr)
{
	if(flag == 'G')
		return _id_G(user, flagn);
	if(flag == 'g')
		return _id_g(user, flagn, flagr);
	if(flag == 'u')
		return _id_u(user, flagn, flagr);
	return _id_all(user);
}

/* _id_error */
static int _id_error(char const * message, int ret)
{
	fputs("id: ", stderr);
	perror(message);
	return ret;
}

/* _id_G */
static int _id_G(char const * user, int flagn)
{
	struct group * gr;
	char * u;
	char ** p;

	if(user == NULL)
	{
		if((gr = getgrgid(getegid())) == NULL)
			return _id_error("getgrgid", 1);
		if(getegid() != getgid())
		{
			if(flagn == 0)
				printf("%u %u", (unsigned)getegid(),
						(unsigned)getgid());
			else
			{
				fputs(gr->gr_name, stdout);
				if((gr = getgrgid(getgid())) == NULL)
				{
					putchar('\n');
					return _id_error("getgrgid", 1);
				}
				printf(" %s", gr->gr_name);
			}
		}
		else
		{
			if(flagn == 0)
				printf("%u", (unsigned)getgid());
			else
				fputs(gr->gr_name, stdout);
		}
	}
	else
	{
		if((gr = getgrnam(user)) == NULL)
			return _id_error(user, 1);
		if(flagn == 0)
			printf("%u", (unsigned)gr->gr_gid);
		else
			puts(gr->gr_name);
	}
	if((u = strdup(gr->gr_name)) == NULL)
		return _id_error(gr->gr_name, 1);
	setgrent();
	for(gr = getgrent(); gr != NULL; gr = getgrent())
	{
		for(p = gr->gr_mem; p != NULL && *p != NULL; p++)
		{
			if(strcmp(u, *p) == 0)
			{
				if(flagn == 0)
					printf(" %u", (unsigned)gr->gr_gid);
				else
					printf(" %s", gr->gr_name);
			}
		}
	}
	putchar('\n');
	endgrent();
	free(u);
	return 0;
}

/* _id_g */
static int _id_g(char const * user, int flagn, int flagr)
{
	struct group * gr;

	if(user == NULL)
	{
		if(flagn == 0)
		{
			printf("%u\n", flagr ? (unsigned)getegid()
					: (unsigned)getgid());
			return 0;
		}
		if((gr = getgrgid(flagr ? getegid() : getgid())) == NULL)
			return _id_error("getgrgid", 1);
		printf("%s\n", gr->gr_name);
		return 0;
	}
	if((gr = getgrnam(user)) == NULL)
		return _id_error(user, 1);
	if(flagn == 0)
		printf("%u\n", (unsigned)gr->gr_gid);
	else
		printf("%s\n", gr->gr_name);
	return 0;
}

/* _id_u */
static int _id_u(char const * user, int flagn, int flagr)
{
	struct passwd * passwd;

	if(user == NULL)
	{
		if(flagn == 0)
		{
			printf("%u\n", flagr ? geteuid() : getuid());
			return 0;
		}
		if((passwd = getpwuid(flagr ? geteuid() : getuid())) == NULL)
			return _id_error("getpwuid", 1);
		printf("%s\n", passwd->pw_name);
		return 0;
	}
	if((passwd = getpwnam(user)) == NULL)
		return _id_error(user, 1);
	if(flagn == 0)
		printf("%u\n", passwd->pw_uid);
	else
		printf("%s\n", passwd->pw_name);
	return 0;
}

/* _id_all */
static struct group * _print_gid(gid_t gid);

static int _id_all(char const * user)
{
	struct passwd * pw;
	struct group * gr;
	char * u;
	char ** p;

	if(user == NULL)
	{
		if((pw = getpwuid(getuid())) == NULL)
		{
			putchar('\n');
			return _id_error("getpwuid", 1);
		}
		printf("uid=%u(%s) ", (unsigned)pw->pw_uid, pw->pw_name);
		if((gr = _print_gid(pw->pw_gid)) == NULL)
			return 1;
		if((u = strdup(gr->gr_name)) == NULL)
		{
			putchar('\n');
			return _id_error(gr->gr_name, 1);
		}
		if(geteuid() != getuid())
		{
			if((pw = getpwuid(geteuid())) == NULL)
			{
				putchar('\n');
				return _id_error("getpwuid", 1);
			}
			printf(" euid=%u(%s) e", (unsigned)pw->pw_uid,
					pw->pw_name);
			if(_print_gid(pw->pw_gid) == NULL)
				return 1;
		}
	}
	else
	{
		if((pw = getpwnam(user)) == NULL)
			return _id_error(user, 1);
		printf("uid=%u(%s) ", (unsigned)pw->pw_uid, pw->pw_name);
		if((gr = _print_gid(pw->pw_gid)) == NULL)
			return 1;
		if((u = strdup(gr->gr_name)) == NULL)
		{
			putchar('\n');
			return _id_error(gr->gr_name, 1);
		}
	}
	printf("%s%u(%s)", " groups=", (unsigned)pw->pw_gid, u);
	setgrent();
	for(gr = getgrent(); gr != NULL; gr = getgrent())
		for(p = gr->gr_mem; p != NULL && *p != NULL; p++)
			if(strcmp(u, *p) == 0)
				printf(",%u(%s)", (unsigned)gr->gr_gid,
						gr->gr_name);
	putchar('\n');
	endgrent();
	free(u);
	return 0;
}

/* _print_gid */
static struct group * _print_gid(gid_t gid)
{
	struct group * gr;

	if((gr = getgrgid(gid)) == NULL)
	{
		putchar('\n');
		_id_error("getgrgid", 0);
	}
	else
		printf("gid=%u(%s)", gr->gr_gid, gr->gr_name);
	return gr;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: id [-Ggu][-nr] [user]\n\
  -G	Output all different group IDs\n\
  -g	Output only the effective group ID\n\
  -u	Output only the effective user ID\n\
  -n	Output the name as a string\n\
  -r	Output the real ID instead of the effective ID\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int flag = 0;
	int flagn = 0;
	int flagr = 0;

	while((o = getopt(argc, argv, "Ggunr")) != -1)
		switch(o)
		{
			case 'G':
			case 'g':
			case 'u':
				flag = o;
				break;
			case 'n':
				flagn = 1;
				break;
			case 'r':
				flagr = 1;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _id(NULL, flag, flagn, flagr) == 0 ? 0 : 2;
	if(optind + 1 == argc)
		return _id(argv[optind], flag, flagn, flagr) == 0 ? 0 : 2;
	return _usage();
}
