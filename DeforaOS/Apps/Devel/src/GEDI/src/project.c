/* project.c */



#include <stdlib.h>
#include "project.h"


/* Project */
/* project_new */
Project * project_new(void)
{
	Project * p;

	if((p = malloc(sizeof(Project))) == NULL)
		return NULL;
	return p;
}

/* project_delete */
void project_delete(Project * project)
{
	free(project);
}


/* useful */
/* project_properties */
void project_properties(Project * project)
{
	/* FIXME */
}
