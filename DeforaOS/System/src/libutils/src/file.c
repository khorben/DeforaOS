/* file.c */



#include <stdlib.h>
#include "file.h"


/* File */
/* file_new */
File * file_new(char * filename, char * mode)
{
	File * file;

	if((file = malloc(sizeof(File))) == NULL)
		return NULL;
	if((file->fp = fopen(filename, mode)) == NULL)
	{
		file_delete(file);
		return NULL;
	}
	return file;
}

/* file_new_from_pointer */
File * file_new_from_pointer(FILE * fp)
{
	File * file;

	if((file = malloc(sizeof(File))) == NULL)
		return NULL;
	file->fp = fp;
	return file;
}

/* file_delete */
void file_delete(File * file)
{
	if(file->fp != NULL)
		fclose(file->fp);
	free(file);
}


/* useful */
/* file_get_line */
char * file_get_line(File * file)
{
#define FGL_BUFSIZ 80
	char * str;
	char * p;
	int len = FGL_BUFSIZ;
	int pos = 0;
	int c;

	if((str = malloc(sizeof(char) * FGL_BUFSIZ + 1)) == NULL)
		return NULL;
	while((c = fgetc(file->fp)) != EOF && c != '\n')
	{
		str[pos++] = c;
		if(pos == len)
		{
			if((p = realloc(str, (len + FGL_BUFSIZ) * sizeof(char)))
					== NULL)
			{
				free(str);
				return NULL;
			}
		}
	}
	str[pos] = '\0';
	return str;
}
