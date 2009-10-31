/* $Id$ */



#ifndef Directory_H
# define Directory_H

# include <stdint.h>
# include <System.h>


/* types */
typedef Buffer * BUFFER;
typedef int32_t INT32;
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
INT32 Directory_register(STRING title, BUFFER csr, BUFFER_OUT x509);

#endif /* !Directory_H */
