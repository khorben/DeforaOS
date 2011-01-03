/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of Makepasswd.
 *
 * Makepasswd is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Makepasswd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Makepasswd; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/* TODO:
 * - I wrote this code a while ago, needs review */



#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../config.h"

/* md5 */
#include "global.h"
#include "md5.h"


/* variables */
int encryption;
int length;
int max;
int min;
int number;
char * password = NULL;
char * string = NULL;			/* string to get allowed characters from */
char string_default[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
int stringn;				/* length of string */
/* md5 */
char md5out[33];			/* MD5 output buffer */
/* des */
/* common to des and shmd5 */
char salt_string[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789./";
int salt_stringn;				/* length of salt_string */
char * salt;


/* options */
enum { CHARACTERS = 0, ENCRYPT, LENGTH, MAX, MIN, NUMBER, PASSWORD, SALT, HELP, VER, NONE };
struct options
{
	char shortarg;
	char* longarg;
	int defaut;
	char* description;
};
struct options args[] = {
	{ 'c', "chars",    -1, "string of allowed characters (A..Za..z0..9)" },
	{ 'e', "encrypt",  -1, "encryption algorithm {none,base64,des,md5,shmd5} (none)" },
	{ 'l', "length",   -1, "password length"			},
	{ 'M', "max",       8, "password maximum length"		},
	{ 'm', "min",       8, "password minimum length"		},
	{ 'n', "number",    1, "number of passwords to generate"	},
	{ 'p', "password", -1, "password to use"			},
	{ 's', "salt",     -1, "salt to use (random)"			},
	{ 'h', "help",     -1, "display this help screen"		},
	{ 'V', "version",  -1, "display program version"		}
};
enum { EB64 = 0, EMD5, EDES, ESHMD5, ENONE };
char* options_encryption[] = { "base64", "md5", "des", "shmd5", "none", NULL };



/* functions */
/* base64 */
void base64(char string[]);
/* md5 */
void md5in(char buffer[], unsigned int length);
/* des */
char* desin(char buffer[], char salt[]);
/* shadow md5 */
char* shmd5in(char buffer[], char salt[]);
/* help */
static int _usage(void);
static void _version(void);


/* main */
int main(int argc, char* argv[])
{
	int i, j, l;
	char * str;
	struct timeval tv;
	int fd;
	short int s;

	/* initializations */
	encryption = ENONE;
	length = args[LENGTH].defaut;
	min = args[MIN].defaut;
	max = args[MAX].defaut;
	number = args[NUMBER].defaut;
	/* md5 */
	md5out[32] = '\0';
	/* des */
	salt_stringn = strlen(salt_string);
	salt = NULL;

	/* parse */
	for(i = 1; i < argc; i+=2)
	{
		/* check syntax */
		if(argv[i][0] != '-' || i+1 == argc)
		{
			/* version */
			if(strcmp(argv[i], "-V") == 0 ||
					strcmp(argv[i], "--version") == 0)
				_version();
			else /* error */
				_usage();
			return 1;
		}

		/* detect argument */
		if(argv[i][1] != '-' && argv[i][2] == '\0') /* short */
			for(j = 0; j < NONE &&
					argv[i][1] != args[j].shortarg; j++);
		else if(argv[i][1] == '-') /* long */
			for(j = 0; j < NONE &&
					strcmp(&argv[i][2], args[j].longarg) != 0; j++);
		else /* error e.g. -hn 1 */
			return _usage();

		/* handle */
		switch(j)
		{
			case CHARACTERS:
				/* remember new string */
				string = argv[i+1];
				break;
			case ENCRYPT:
				for(j = 0; options_encryption[j] &&
						strcmp(options_encryption[j], argv[i+1]) != 0;
						j++);
				if(options_encryption[j] == NULL)
					return _usage();
				else
					encryption = j;
				break;
			case LENGTH:
				j = atoi(argv[i+1]);
				min = j;
				max = j;
				break;
			case MAX:
				j = atoi(argv[i+1]);
				max = j;
				if(min > max)
					min = max;
				break;
			case MIN:
				j = atoi(argv[i+1]);
				min = j;
				if(min > max)
					max = min;
				break;
			case NUMBER:
				number = atoi(argv[i+1]);
				break;
			case PASSWORD:
				password = argv[i+1];
				break;
			case SALT:
				salt = argv[i+1];
				break;
			default:
				return _usage();
		}
	}

	/* check consistency */
	/* password length */
	if(number < 1 || max < min || min < 1)
		return _usage();
	/* password characters */
	if(string == NULL)
	{
		string = string_default;
		stringn = strlen(string);
	}
	else
		stringn = strlen(string);
	/* salt */
	if(salt)
	{
		int n;
		
		n = strlen(salt);

		/* length */
		switch(encryption)
		{
			case EDES:
				if(n != 2)
				{
					_usage();
					puts("DES salt is 2 characters long, chosen from \"A..Za..z0..9./\"");
					return 1;
				}
				break;
			case ESHMD5:
				if(n > 8)
				{
					_usage();
					puts("SHMD5 salt is up to 8 characters long, chosen from \"A..Za..z0..9./\"");
					exit(1);
				}
				break;
			default:
				_usage();
				puts("Salt is only used with DES and SHMD5 encryptions");
				exit(1);
				break;
		}

		/* content */
		for(i = 0; i < n; i++)
		{
			int j;
			for(j = 0; j < salt_stringn && salt[i] != salt_string[j]; j++);
			if(j == salt_stringn)
			{
				_usage();
				puts("Salt is chosen from \"A..Za..z0..9./\"");
				exit(1);
			}
		}
	}

	/* initialize random seed */
	if(gettimeofday(&tv, NULL) != 0)
	{
		perror("gettimeofday");
		exit(2);
	}
	if((fd = open("/dev/random", O_RDONLY)) < 0)
	{
		perror("/dev/random");
		exit(2);
	}
	read(fd, &s, sizeof(s));
	close(fd);
	srand(tv.tv_sec ^ tv.tv_usec ^ getuid() ^ (getpid() << 16) ^ s);

	/* go for it */
	if(password != NULL)
	{
		str = password;
		max = strlen(str);
		min = max;
		l = min;
	}
	else
	{
		str = malloc(sizeof(char) * (max + 1));
		if(str == NULL)
		{
			perror("malloc");
			exit(1);
		}

		l = min;
		str[max] = '\0';
	}

	for(i = 0; i < number; i++)
	{
		/* choose random length if necessary */
		if(min != max)
		{
			l = min + (rand() % (max + 1 - min));
			str[l] = '\0';
		}

		/* create password */
		if(password == NULL)
			for(j = 0; j < l; j++)
				str[j] = string[rand() % stringn];

		switch(encryption)
		{
			case ENONE:
				puts(str);
				break;
			case EB64:
				fputs(str, stdout);
				for(j = strlen(str); j <= max; j++)
					printf(" ");
				base64(str);
				printf("\n");
				break;
			case EMD5:
				md5in(str, l);
				printf("%s", str);
				for(; l <= max; l++)
					printf(" ");
				puts(md5out);
				l = min;
				break;
			case EDES:
				printf("%s", str);
				for(; l <= max; l++)
					printf(" ");
				if(salt != NULL)
					puts(desin(str, salt));
				else
				{
					char salt[2];
					salt[0] = salt_string[rand() % salt_stringn];
					salt[1] = salt_string[rand() % salt_stringn];
					puts(desin(str, salt));
				}
				l = min;
				break;
			case ESHMD5:
				printf("%s", str);
				for(; l <= max; l++)
					printf(" ");
				if(salt != NULL)
					puts(shmd5in(str, salt));
				else
				{
					int n = rand() % 9;
					{
						char s[n + 1];
						s[n--] = '\0';
						for(; n >= 0; n--)
							s[n] = salt_string[rand() % salt_stringn];
						puts(shmd5in(str, s));
					}
				}
				l = min;
				break;
		}
	}

	if(password == NULL)
	{
		memset(str, 0, strlen(str));
		free(str);
	}

	return 0;
}


/* functions */
/* base64 */
void base64(char string[])
{
	unsigned int len;
	unsigned int i;
	unsigned int j;
	unsigned char bufi[3];
	unsigned char bufo[4];
	char conv[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	len = strlen(string);
	for(i = 0; i < len;)
	{
		j = 0;
		for(; j < 3 && i < len; j++)
			bufi[j] = string[i++];
		for(; j < 3; j++)
			bufi[j] = 0;
		bufo[0] = bufi[0] >> 2;
/*		bufo[1] = ((bufi[0] << 6) >> 2) + (bufi[1] >> 4); */
		bufo[1] = bufi[0] << 6;
		bufo[1] = bufo[1] >> 2;
		bufo[2] = bufi[1] >> 4;
		bufo[1] += bufo[2];
/*		bufo[2] = ((bufi[1] << 4) >> 2) + (bufi[2] >> 6); */
		bufo[2] = bufi[1] << 4;
		bufo[2] = bufo[2] >> 2;
		bufo[3] = bufi[2] >> 6;
		bufo[2] += bufo[3];
		bufo[3] = bufi[3] << 2;
		bufo[3] = bufo[3] >> 2;
		for(j = 0; j < 4; j++)
			printf("%c", conv[bufo[j]]);
	}
}

/* md5 */
void md5in(char buffer[], unsigned int length)
{
	unsigned char digest[16];
	unsigned short i;
	unsigned char j;
	MD5_CTX c;

	/* call md5 implementation from RSA Data Security, Inc. */
	MD5Init(&c);
	MD5Update(&c, buffer, length);
	MD5Final(digest, &c);

	/* from RFC 2617 */
	for (i = 0; i < 16; i++)
	{
		j = (digest[i] >> 4) & 0xf;
		if(j <= 9)
			md5out[i*2] = (j + '0');
		else
			md5out[i*2] = (j + 'a' - 10);
		j = digest[i] & 0xf;
		if(j <= 9)
			md5out[i*2+1] = (j + '0');
		else
			md5out[i*2+1] = (j + 'a' - 10);
	}
}

/* des */
char *crypt(const char *key, const char *salt);
char* desin(char buffer[], char salt[])
{
	return crypt(buffer, salt);
}

/* shadow md5 */
char* shmd5in(char buffer[], char salt[])
{
	char str[4 + strlen(salt)];
	sprintf(str, "$1$%s", salt);
	return crypt(buffer, str);
}

/* usage */
static int _usage(void)
{
	int i;

	printf("Usage: makepasswd [option [value]] ...\n");
	for(i = 0; i < NONE; i++)
	{
		printf("\t-%c, --%s\t%s", args[i].shortarg, args[i].longarg, args[i].description);
		if(args[i].defaut != -1)
			printf(" (%d)", args[i].defaut);
		printf("\n");
	}
	return 1;
}

/* version */
static void _version(void)
{
	printf("%s %s\n", PACKAGE, VERSION);
}
