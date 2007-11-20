/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix others */
/* others is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * others is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with others; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>



/* login */
static int _login_error(char const * message, int ret);
static int _login_do(char const * user);

static int _login(char const * user)
{
	int i;

	for(i = 0; i < 3; i++)
		if(_login_do(user) != 0)
			return 1;
	return 0;
}

static int _login_error(char const * message, int ret)
{
	fputs("login: ", stderr);
	perror(message);
	return ret;
}

static char const * _do_ask_username(void);
static char const * _do_ask_password(void);

static int _login_do(char const * user)
{
	struct passwd * pw;
	char const * password = NULL;

	if(user == NULL)
		user = _do_ask_username();
	if(user == NULL)
		return 1;
	pw = getpwnam(user);
	if(getuid() != 0
			&& (password = _do_ask_password()) == NULL)
		return 1;
	if(pw == NULL		/* user does not exist */
			|| (password != NULL && (pw->pw_passwd == NULL
					|| pw->pw_passwd[0] == '*'
					|| strcmp(password, pw->pw_passwd)
					!= 0))) /* wrong/encrypted password */
	{
		fputs("Login failed\n", stderr);
		sleep(3);
		return 0;
	}
	if(setgid(pw->pw_gid) != 0)
		return _login_error("setgid", 1);
	if(setuid(pw->pw_uid) != 0)
		return _login_error("setuid", 1);
	/* FIXME sanitize environment */
	execl(pw->pw_shell, pw->pw_shell, NULL);
	exit(2);
	return 0;
}

static char const * _do_ask_username(void)
{
	static char buf[256];
	size_t len;
	int c;

	fputs("login: ", stderr);
	if(fgets(buf, sizeof(buf), stdin) == NULL)
	{
		if(!feof(stdin))
			_login_error("fgets", 1);
		return NULL;
	}
	if((len = strlen(buf)) == 0
			|| buf[0] == '\n')
		return "";
	if(buf[len - 1] == '\n')
	{
		buf[len - 1] = '\0';
		return buf;
	}
	while((c = fgetc(stdin)) != EOF		/* flush line */
			&& c != '\n');
	return NULL;
}

static char const * _do_ask_password(void)
{
	static char buf[256];
	size_t len;
	int c;

	fputs("password: ", stderr);
	if(fgets(buf, sizeof(buf), stdin) == NULL)
	{
		_login_error("fgets", 1);
		return NULL;
	}
	len = strlen(buf);
	if(len == 0)
		return NULL;
	if(buf[len - 1] == '\n')
	{
		buf[len - 1] = '\0';
		return buf;
	}
	while((c = fgetc(stdin)) != EOF		/* flush line */
			&& c != '\n');
	return NULL;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: login [user]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char * user = NULL;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind != argc
			&& optind + 1 != argc)
		return _usage();
	return _login(user) ? 0 : 2;
}
