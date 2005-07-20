/* tokencode.c */



#include <stdlib.h>
#include "tokencode.h"


/* TokenCode */
char * sTokenCode[TC_LAST+1] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"\n",
	NULL,
	"&&",
	"||",
	";;",
	"<<",
	">>",
	"<&",
	">&",
	"<>",
	"<<-",
	">|",
	"&",
	"|",
	";",
	"<",
	">",
	"if",
	"then",
	"else",
	"elif",
	"fi",
	"do",
	"done",
	"case",
	"esac",
	"while",
	"until",
	"for",
	"{",
	"}",
	"!",
	"in",
	NULL
};


/* TokenCode sets */
TokenCode CS_AND_OR[]		= {
	TC_RW_BANG,
	/* command */
	TC_TOKEN,
	TC_NULL
};
TokenCode CS_CMD_NAME[]		= {
	TC_WORD,
	TC_NULL
};
TokenCode CS_CMD_PREFIX[]	= {
	/* io_redirect */
	TC_IO_NUMBER,
	/* ASSIGNMENT WORD */
	TC_ASSIGNMENT_WORD,
	TC_NULL
};
TokenCode CS_CMD_SUFFIX[]	= {
	/* io_redirect */
	TC_IO_NUMBER,
	/* WORD */
	TC_WORD,
	TC_NULL
};
TokenCode CS_CMD_WORD[]		= {
	TC_WORD,
	TC_NULL
};
TokenCode CS_COMMAND[]		= {
	/* simple_command */
	TC_TOKEN,
	/* compound_command */
	TC_RW_LBRACE, /* SUBSHELL "(", */ TC_RW_FOR, TC_RW_CASE, TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL,
	/* function_definition */
	TC_NULL
};
TokenCode CS_COMPOUND_COMMAND[]	= {
	TC_RW_LBRACE, /* SUBSHELL "(", */
	TC_RW_FOR, TC_RW_CASE, TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL,
	TC_NULL
};
TokenCode CS_COMPOUND_LIST[]	= {
	/* term */
	TC_RW_BANG,
	TC_TOKEN,
	/* newline_list */
	TC_NEWLINE,
	TC_NULL
};
TokenCode CS_DO_GROUP[]		= {
	/* Do */
	TC_RW_DO,
	TC_NULL
};
TokenCode CS_FUNCTION_BODY[]	= {
	/* compound_command */
	TC_RW_LBRACE, /* SUBSHELL "(", */
	TC_RW_FOR, TC_RW_CASE, TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL,
	TC_NULL
};
TokenCode CS_FUNCTION_DEFINITION[] = {
	/* fname */
	TC_NAME,
	TC_NULL
};
TokenCode CS_IN[]		= {
	TC_RW_IN,
	TC_NULL
};
TokenCode CS_IO_FILE[]		= {
	/* '<' */
	TC_OP_LESS,
	/* LESSAND */
	TC_OP_LESSAND,
	/* '>' */
	TC_OP_GREAT,
	/* GREATAND */
	TC_OP_GREATAND,
	/* DGREAT */
	TC_OP_DGREAT,
	/* LESSGREAT */
	TC_OP_LESSGREAT,
	/* CLOBBER */
	TC_OP_CLOBBER,
	TC_NULL
};
TokenCode CS_IO_HERE[]		= {
	/* DLESS */
	TC_OP_DLESS,
	/* DLESSDASH */
	TC_OP_DLESSDASH,
	TC_NULL
};
TokenCode CS_IO_REDIRECT[]	= {
	/* io_file */
	TC_OP_LESS, TC_OP_LESSAND, TC_OP_GREAT, TC_OP_GREATAND,
	TC_OP_DGREAT, TC_OP_LESSGREAT, TC_OP_CLOBBER,
	/* IO_NUMBER */
	TC_IO_NUMBER,
	/* io_here */
	TC_OP_DLESS, TC_OP_DLESSDASH,
	TC_NULL
};
TokenCode CS_LINEBREAK[]	= {
	TC_NEWLINE,
	TC_NULL
}; /* FIXME */
TokenCode CS_LIST[]		= {
	/* and_or */
	TC_RW_BANG, TC_TOKEN,
	/* separator_op */
	TC_OP_AMPERSAND, TC_OP_SEMICOLON,
	TC_NULL
};
TokenCode CS_NAME[]		= {
	TC_NAME,
	TC_NULL
};
TokenCode CS_NEWLINE_LIST[]	= {
	TC_NEWLINE,
	TC_NULL
};
TokenCode CS_PIPE_SEQUENCE[]	= {
	/* command */
	TC_TOKEN,
	TC_RW_LBRACE, /* SUBSHELL "(", */ TC_RW_FOR, TC_RW_CASE, TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL,
	TC_NULL
};
TokenCode CS_PIPELINE[]		= {
	/* Bang */
	TC_RW_BANG,
	/* pipe_sequence */
	TC_NULL
};
TokenCode CS_REDIRECT_LIST[]	= {
	TC_NULL
}; /* FIXME */
TokenCode CS_SEPARATOR[]		= {
	TC_NEWLINE,
	TC_NULL
}; /* FIXME */
TokenCode CS_SEPARATOR_OP[]	= {
	TC_OP_AMPERSAND,
	TC_OP_SEMICOLON,
	TC_NULL
};
TokenCode CS_SEQUENTIAL_SEP[]	= {
	TC_NULL
}; /* FIXME */
TokenCode CS_SIMPLE_COMMAND[]	= {
	/* cmd_prefix */
	TC_IO_NUMBER,
	/* cmd_name */
	TC_WORD,
	TC_NULL
};
TokenCode CS_TERM[]		= {
	/* and_or */
	TC_RW_BANG,
	TC_TOKEN,
	TC_NULL
};
TokenCode CS_WORDLIST[]		= {
	TC_WORD,
	TC_NULL
};
