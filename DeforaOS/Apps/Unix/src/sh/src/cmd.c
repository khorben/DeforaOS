/* cmd.c */



#include <stdlib.h>
#include <stdio.h>
#include "cmd.h"


/* cmd_prompt */
void cmd_prompt(void)
{
	char * prompt;

	if((prompt = getenv("PS1")) == NULL)
		fprintf(stderr, "$ ");
	else
		fprintf(stderr, "%s", prompt);
}
