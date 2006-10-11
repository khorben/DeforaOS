/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/* types */
typedef enum _pwd_flag {
	PWD_P = 1,
	PWD_LP = 2
} pwd_flag;


/* pwd */
static int _pwd(pwd_flag pf)
{
	char * pwd;
	char buf[256];
	char * p;

	pwd = getenv("PWD");
	if(pf == PWD_LP && pwd != NULL && *pwd == '/')
	{
		for(p = pwd; *p != '\0'; p++)
		{
			if(*p != '/')
				continue;
			if(*++p != '.')
				continue;
			if(*++p == '\0' || *p == '/')
			{
				p--;
				break;
			}
			if(*p != '.')
				continue;
			if(*++p == '\0' || *p == '/')
			{
				p--;
				break;
			}
		}
		if(*p == '\0')
		{
			printf("%s\n", pwd);
			return 0;
		}
	}
	if(pwd != NULL)
	{
		/* FIXME check for symlinks */
	}
	else
	{
		if(getcwd(buf, 255) == NULL)
		{
			fprintf(stderr, "%s", "pwd: ");
			perror("getcwd");
			return 2;
		}
		buf[255] = '\0';
		pwd = buf;
	}
	printf("%s\n", pwd);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: pwd [-L|-P]\n\
  -L    avoid \".\" or \"..\" filenames\n\
  -P    avoid symbolic links\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	pwd_flag pf = PWD_LP;
	int o;

	while((o = getopt(argc, argv, "LP")) != -1)
		switch(o)
		{
			case 'L':
				pf = PWD_LP;
				break;
			case 'P':
				pf = PWD_P;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return _pwd(pf);
}
