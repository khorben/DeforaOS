/* tokencode.h */



#ifndef _TOKENCODE_H
# define _TOKENCODE_H


/* types */
typedef enum _TokenCode {
	TC_EOI = 0,
	TC_TOKEN,
	TC_WORD,
	TC_ASSIGNMENT_WORD,
	TC_NAME,
	TC_NEWLINE,		/* '\n' */
	TC_IO_NUMBER,
	TC_OP_AND_IF,		/* "&&"    */
	TC_OP_OR_IF,		/* "||"    */
	TC_OP_DSEMI,		/* ";;"    */
	TC_OP_DLESS,		/* "<<"    */
	TC_OP_DGREAT,		/* ">>"    */
	TC_OP_LESSAND,		/* "<&"    */
	TC_OP_GREATAND,		/* ">&"    */
	TC_OP_LESSGREAT,	/* "<>"    */
	TC_OP_DLESSDASH,	/* "<<-"   */
	TC_OP_CLOBBER,		/* ">|"    */
	TC_OP_AMPERSAND,	/* "&"     */
	TC_OP_BAR,		/* "|"     */
	TC_OP_SEMICOLON,	/* ";"     */
	TC_OP_LESS,		/* "<"     */
	TC_OP_GREAT,		/* ">"     */
	TC_RW_IF,	/* "if"    */
	TC_RW_THEN,	/* "then"  */
	TC_RW_ELSE,	/* "else"  */
	TC_RW_ELIF,	/* "elif"  */
	TC_RW_FI,	/* "fi"    */
	TC_RW_DO,	/* "do"    */
	TC_RW_DONE,	/* "done"  */
	TC_RW_CASE,	/* "case"  */
	TC_RW_ESAC,	/* "esac"  */
	TC_RW_WHILE,	/* "while" */
	TC_RW_UNTIL,	/* "until" */
	TC_RW_FOR,	/* "for"   */
	TC_RW_LBRACE,	/* "{"     */
	TC_RW_RBRACE,	/* "}"     */
	TC_RW_BANG,	/* "!"     */
	TC_RW_IN,	/* "in"    */
	TC_NULL
} TokenCode;

extern char * sTokenCode[TC_NULL];


/* TokenCode sets */
extern TokenCode CS_AND_OR[];
extern TokenCode CS_CMD_NAME[];
extern TokenCode CS_CMD_PREFIX[];
extern TokenCode CS_CMD_SUFFIX[];
extern TokenCode CS_CMD_WORD[];
extern TokenCode CS_COMPOUND_COMMAND[];
extern TokenCode CS_COMPOUND_LIST[];
extern TokenCode CS_DO_GROUP[];
extern TokenCode CS_FUNCTION_BODY[];
extern TokenCode CS_FUNCTION_DEFINITION[];
extern TokenCode CS_IN[];
extern TokenCode CS_IO_FILE[];
extern TokenCode CS_IO_HERE[];
extern TokenCode CS_IO_NUMBER[];
extern TokenCode CS_IO_REDIRECT[];
extern TokenCode CS_LINEBREAK[];
extern TokenCode CS_LIST[];
extern TokenCode CS_NAME[];
extern TokenCode CS_NEWLINE_LIST[];
extern TokenCode CS_PIPE_SEQUENCE[];
extern TokenCode CS_PIPELINE[];
extern TokenCode CS_REDIRECT_LIST[];
extern TokenCode CS_SEPARATOR[];
extern TokenCode CS_SEPARATOR_OP[];
extern TokenCode CS_SEQUENTIAL_SEP[];
extern TokenCode CS_SIMPLE_COMMAND[];
extern TokenCode CS_TERM[];
extern TokenCode CS_WORDLIST[];

#endif /* !_TOKENCODE_H */
