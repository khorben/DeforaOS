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

/* file_delete */
void file_delete(File * file)
{
	if(file->fp != NULL)
		fclose(file->fp);
	free(file);
}
