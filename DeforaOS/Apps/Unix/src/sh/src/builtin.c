/* builtin.c */



#include <unistd.h>
#include <stdio.h>
#include "builtin.h"


/* builtin_exec */
int builtin_exec(int argc, char * argv[])
{
	if(argc == 0)
		return 0;
	/* FIXME */
	return 0;
}


/* builtin_read */
int builtin_read(int argc, char * argv[])
{
	/* FIXME fgets/realloc */
	return 0;
}


/* set */
static int _set_usage(void);
static int _set_list(void);
int builtin_set(int argc, char * argv[])
{
	if(argc == 0)
		return _set_list();
	/* FIXME */
	return _set_usage();
}

static int _set_usage(void)
{
	/* FIXME */
	fprintf(stderr, "%s", "Usage: set\n");
	return 1;
}

static int _set_list(void)
{
	char ** e;

	for(e = environ; *e != NULL; e++)
		printf("%s\n", *e);
	return 0;
}
