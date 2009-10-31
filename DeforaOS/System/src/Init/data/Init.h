/* $Id$ */



#ifndef Init_H
# define Init_H

# include <stdint.h>
# include <System.h>


/* types */
typedef Buffer * BUFFER;
typedef int16_t INT16;
typedef int32_t INT32;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef String const * STRING;

typedef BUFFER BUFFER_IN;
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


/* functions */
UINT16 Init_login(STRING username);
UINT16 Init_logout();
UINT16 Init_register(STRING, UINT16);
UINT16 Init_get_session(STRING);
INT32 Init_get_profile(STRING_OUT profile);
INT32 Init_set_profile(STRING profile);

#endif /* !Init_H */
