/* $Id$ */



#ifndef Probe_H
# define Probe_H

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
UINT32 Probe_uptime();
INT32 Probe_load(UINT32_OUT load_1, UINT32_OUT load_5, UINT32_OUT load_15);
INT32 Probe_ram(UINT32_OUT total, UINT32_OUT free, UINT32_OUT shared, UINT32_OUT buffer);
INT32 Probe_swap(UINT32_OUT total, UINT32_OUT free);
UINT32 Probe_users();
UINT32 Probe_procs();
UINT32 Probe_ifrxbytes(STRING interface);
UINT32 Probe_iftxbytes(STRING interface);
UINT32 Probe_voltotal(STRING volume);
UINT32 Probe_volfree(STRING volume);

#endif /* !Probe_H */
