/* common.h */



#ifndef COMMON_H
# define COMMON_H


/* AppServer/AppClient */
# define ASC_BUFSIZE 65536
# define ASC_PORT_SESSION 4242

typedef enum _ASCCallType {
	ASC_INT8, ASC_INT16, ASC_INT32, ASC_INT64,
	ASC_UINT8, ASC_UINT16, ASC_UINT32, ASC_UINT64,
	ASC_BUFFER
} ASCCallType;

typedef struct _ASCCall
{
	ASCCallType type;
	char * name;
	ASCCallType * args;
	int args_cnt;
} ASCCall;

int asc_send_call(ASCCall * call, char * buf, int buflen, void ** args);

#endif /* !COMMON_H */
