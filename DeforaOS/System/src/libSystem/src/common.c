/* common.c */



#include <stdint.h>

#include "common.h"


/* AppServer/AppClient */
/* private */
/* functions */
static int _send_buffer(char * data, int datalen, char * buf, int buflen,
		int * pos)
{
	if(*pos + datalen > buflen)
		return 1;
	memcpy(&buf[*pos], data, datalen);
	*pos += datalen;
	return 0;
}


static int _send_string(char * string, char * buf, int buflen, int * pos)
{
	int i = 0;

	while(*pos < buflen)
	{
		buf[*pos] = string[i++];
		(*pos)++;
		if(string[i] == '\0')
			return 0;
	}
	return 1;
}


/* public */
int asc_send_call(ASCCall * call, char buf[], int buflen, void ** args)
{
	int pos = 0;
	int i;
	int size;

	if(_send_string(call->name, buf, buflen, &pos) != 0)
		return -1;
	for(i = 0; i < call->args_cnt; i++)
	{
		switch(call->args[i])
		{
			case ASC_INT8:
			case ASC_UINT8:
				size = sizeof(int8_t);
				break;
			case ASC_INT16:
			case ASC_UINT16:
				size = sizeof(int16_t);
				break;
			case ASC_INT32:
			case ASC_UINT32:
				size = sizeof(int32_t);
				break;
			case ASC_INT64:
			case ASC_UINT64:
				size = sizeof(int64_t);
				break;
		}
		if(_send_buffer(args[i], size, buf, buflen, &pos) != 0)
			return 1;
	}
	return 0;
}
