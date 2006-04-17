/* string.c */



#include <stdlib.h>
#include <string.h>
#include "System.h"


/* String */
String * string_new(String const * string)
{
	String * str;
	int length = string_length(string);

	if((str = malloc(length + 1)) == NULL)
		return NULL;
	strcpy(str, string);
	return str;
}


void string_delete(String * string)
{
	free(string);
}


/* useful */
/* string_append */
int string_append(String ** string, String const * append)
{
	char * p;
	int length = string_length(*string);

	if((p = realloc(*string, length + string_length(append) + 1)) == NULL)
		return 1;
	*string = p;
	strcpy(*string + length, append);
	return 0;
}


/* string_compare */
int string_compare(String const * string, String const * string2)
{
	unsigned char const * u1;
	unsigned char const * u2;

	u1 = string;
	u2 = string2;
	while(*u1 && *u2 && *u1 == *u2)
	{
		u1++;
		u2++;
	}
	return *u1 - *u2;
}


/* string_compare_length */
int string_compare_length(String const * string, String const * string2,
		unsigned int length)
{
	unsigned char const * u1;
	unsigned char const * u2;

	u1 = string;
	u2 = string2;
	while(--length && *u1 && *u2 && *u1 == *u2)
	{
		u1++;
		u2++;
	}
	return *u1 - *u2;
}


/* string_find */
String const * string_find(String const * string, String const * key)
{
	unsigned int len = string_length(key);
	unsigned char const * p;

	for(p = string; *p != '\0' && string_compare_length(key, p, len) != 0;
			p++);
	if(*p == '\0')
		return NULL;
	return p;
}


/* string_length */
int string_length(String const * string)
{
	int length;

	for(length = 0; *string != '\0'; string++)
		length++;
	return length;
}
