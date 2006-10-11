/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <libgen.h>
#include <stdio.h>
#include <string.h>


/* basename */
static int _basename(char * arg, char * suf)
{
	char * str;
	int slen;
	int alen;

	str = basename(arg);
	if(suf != NULL)
	{
		slen = strlen(str);
		alen = strlen(suf);
		if(alen < slen && strcmp(suf, &str[slen - alen]) == 0)
			str[slen - alen] = '\0';
	}
	printf("%s\n", str);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: basename string [suffix]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc == 2)
		return _basename(argv[1], NULL);
	if(argc == 3)
		return _basename(argv[1], argv[2]);
	return _usage();
}
