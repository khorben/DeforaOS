/* code.c */



#include <stdlib.h>
#include "as.h"
#include "arch/arch.h"
#include "code.h"


/* Code */
Code * code_new(char * arch, char * format, char * filename)
{
	Code * c;

	if((c = malloc(sizeof(Code))) == NULL)
	{
		as_error("malloc", 0);
		return NULL;
	}
	if((c->arch = arch_new(arch)) == NULL
			|| (c->format = format_new(format)) == NULL
			|| (c->fp = fopen(filename, "w")) == NULL)
	{
		if(c->arch != NULL)
			arch_delete(c->arch);
		if(c->format != NULL)
			format_delete(c->format);
		if(c->fp != NULL)
			fclose(c->fp);
		else
			as_error(filename, 0);
		free(c);
		return NULL;
	}
	c->filename = filename;
	return c;
}


/* code_delete */
void code_delete(Code * code)
{
	arch_delete(code->arch);
	format_delete(code->format);
	free(code);
}
