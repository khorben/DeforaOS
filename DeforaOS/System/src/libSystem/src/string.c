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
int string_append(String ** string, String * append)
{
	char * p;
	int len = string_length(*string);

	if((p = realloc(string, len + string_length(append) + 1)) == NULL)
	{
		perror("realloc");
		return 1;
	}
	*string = p;
	strcpy(*string + len, append);
	return 0;
}


int string_length(String * string)
{
	int length;

	for(length = 0; *string != '\0'; string++)
		length++;
	return length;
}
