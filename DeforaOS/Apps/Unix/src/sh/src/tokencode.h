/* tokencode.h */



#ifndef __TOKENCODE_H
# define __TOKENCODE_H


/* TokenCode */
typedef enum _TokenCode {
	TC_TOKEN,
	TC_WORD,
	TC_ASSIGNMENT_WORD,
	TC_NAME,
	TC_IONUMBER,
	TC_NEWLINE,	/* '\n'    */
	TC_OP_AND_IF,	/* "&&"    */
	TC_OP_OR_IF,	/* "||"    */
	TC_OP_DSEMI,	/* ";;"    */
	TC_OP_DLESS,	/* "<<"    */
	TC_OP_DGREAT,	/* ">>"    */
	TC_OP_LESSAND,	/* "<&"    */
	TC_OP_GREATAND,	/* ">&"    */
	TC_OP_LESSGREAT,/* "<>"    */
	TC_OP_DLESSDASH,/* "<<-"   */
	TC_OP_CLOBBER,	/* ">|"    */
	TC_OP_AMPERSAND,/* "&"     */
	TC_OP_BAR,	/* "|"     */
	TC_OP_SEMICOLON,/* ";"     */
	TC_OP_LESS,     /* "<"     */
	TC_OP_GREAT,    /* ">"     */
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
	TC_RW_BANG,   	/* "!"     */
	TC_RW_IN,   	/* "in"    */
	TC_ANY,
	TC_NULL
} TokenCode;

extern char * sTokenCode[TC_NULL];

#endif /* __TOKENCODE_H */
