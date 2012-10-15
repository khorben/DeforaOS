/* $Id$ */



#include <stdio.h>
#include <string.h>
#include "Mailer.h"


/* email */
static int _email(char const * progname, char const * name, char const * email,
		char const * str)
{
	int ret = 0;
	char * n;
	char * e;

	n = mailer_helper_get_name(str);
	e = mailer_helper_get_email(str);
	if(strcmp(name, n) != 0)
		fprintf(stderr, "%s: %s: %s (\"%s\")\n", progname, str,
				"The name does not match", n);
	if(strcmp(email, e) != 0)
		fprintf(stderr, "%s: %s: %s (\"%s\")\n", progname, str,
				"The e-mail does not match", e);
	free(e);
	free(n);
	return ret;
}


/* main */
int main(int argc, char * argv[])
{
	int ret = 0;

	ret |= _email(argv[0], "John Doe", "john@doe.com",
			"john@doe.com (John Doe)");
	ret |= _email(argv[0], "John Doe", "john@doe.com",
			"John Doe <john@doe.com>");
	return ret;
}
