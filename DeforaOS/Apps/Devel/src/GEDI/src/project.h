/* project.h */



#ifndef __PROJECT_H
# define __PROJECT_H

# include <libutils.h>


/* types */
typedef struct _Project
{
	Config * config;
} Project;


/* functions */
Project * project_new(void);
void project_delete(Project * project);

#endif /* !__PROJECT_H */
