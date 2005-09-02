/* string.c */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "string.h"


/* String */
String * string_new(String * string)
{
	String * str;
	int length = string_length(string);

	if((str = malloc(length + 1)) == NULL)
		perror("strdup");
	strcpy(str, string);
	return str;
}


void string_delete(String * string)
{
	free(string);
}


/* useful */
/* string_append */
int string_append(String ** string, String * append)
{
	char * p;
	int length = string_length(*string);

	if((p = realloc(string, length + string_length(append) + 1)) == NULL)
	{
		perror("realloc");
		return 1;
	}
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


/* string_length */
int string_length(String * string)
{
	int length;

	for(length = 0; *string != '\0'; string++)
		length++;
	return length;
}
