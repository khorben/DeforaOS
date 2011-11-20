/* $Id$ */



#ifndef VPN_VPN_H
# define VPN_VPN_H

# include <stdint.h>
# include <System.h>


/* types */
typedef Buffer * BUFFER;
typedef double * DOUBLE;
typedef float * FLOAT;
typedef int16_t INT16;
typedef int32_t INT32;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef String const * STRING;
typedef void VOID;

typedef BUFFER BUFFER_IN;

typedef DOUBLE DOUBLE_IN;

typedef FLOAT FLOAT_IN;
typedef INT32 INT32_IN;
typedef UINT32 UINT32_IN;
typedef STRING STRING_IN;

typedef Buffer * BUFFER_OUT;
typedef int32_t * INT32_OUT;
typedef uint32_t * UINT32_OUT;
typedef String ** STRING_OUT;

typedef Buffer * BUFFER_INOUT;
typedef int32_t * INT32_INOUT;
typedef uint32_t * UINT32_INOUT;
typedef String ** STRING_INOUT;


/* constants */
# define VPN_PROTOCOL_IP_TCP	0
# define VPN_EPERM	1
# define VPN_EBADF	9
# define VPN_EPROTO	96


/* calls */
INT32 VPN_close(INT32 fd);
INT32 VPN_connect(UINT32 protocol, STRING name);
INT32 VPN_recv(INT32 fd, BUFFER_IN buf, UINT32 size, UINT32 flags);
INT32 VPN_send(INT32 fd, BUFFER_OUT buf, UINT32 size, UINT32 flags);

#endif /* !VPN_VPN_H */
