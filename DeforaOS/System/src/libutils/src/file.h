/* file.h */



#ifndef ___FILE_H
# define ___FILE_H

# include <stdio.h>


/* types */
typedef struct _File {
	FILE * fp;
} File;


/* functions */
/* file_new */
File * file_new(char * filename, char * mode);
File * file_new_from_pointer(FILE * fp);

/* file_delete */
void file_delete(File * file);

/* useful */
/* file_get_line */
char * file_get_line(File * file);

#endif /* !___FILE_H */
