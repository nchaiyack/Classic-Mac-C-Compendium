/*
 * Written by Aaron Wohl (aw0g+@andrew.cmu.edu), Jan 1990
 * compiler/system specific definitions
 * by Aaron Wohl (n3liw+@cmu.edu)
 * public domain no rights reserved
 */

#ifndef _H_msncomp
#define _H_msncomp

#ifdef vax
#undef msn_ANSI
#define msn_VOID char
#endif

#ifdef THINK_C
#undef msn_ANSI
#define msn_ANSI
#endif

#ifndef msn_VOID
#define msn_VOID void
#endif

#ifndef msn_VOID_PTR
#define msn_VOID_PTR msn_VOID *
#endif

#ifdef msn_ANSI
#define msn_ANSI_ARG(xx_arg) xx_arg
#else
#define msn_ANSI_ARG(xx_arg) ()
#endif

#define msn_TRUE (0==0)
#define msn_FALSE (0==1)

#define msn_imin(xx1,xx2) (((xx1)<(xx2))?(xx1):(xx2))
#define msn_imax(xx1,xx2) (((xx1)>(xx2))?(xx1):(xx2))
#define msn_iabs(xx1) ((xx1)<0)?(-(xx1)):(xx1)

#define msn_NIL 0L

#ifndef msn_INT_MACROS
#define msn_int8 char
#define msn_uint8 unsigned char
#define msn_int16 short int
#define msn_uint16 unsigned short int
#define msn_int32 long
#define msn_uint32 unsigned long
#define msn_uint unsigned int
#define msn_ulong unsigned long
#endif msn_INT_MACROS

#ifdef msn_BOGUS_FAKE_DEFINES
msn_VOID memcpy msn_ANSI_ARG((msn_VOID *dst,msn_VOID *src,long len));
msn_VOID memset msn_ANSI_ARG((msn_VOID *src,int val,long len));
msn_VOID strcpy msn_ANSI_ARG((char *dst,char *src));
msn_VOID_PTR malloc msn_ANSI_ARG((long size));
msn_VOID free msn_ANSI_ARG ((msn_VOID_PTR freeme));
#endif msn_BOGUS_FAKE_DEFINES

#include <string.h>

#ifndef msn_PUBLIC_PROC
#define msn_PUBLIC_PROC
#define msn_PRIVATE_PROC static
#define msn_PRIVATE_DATA static
#define msn_PUBLIC_DATA
#endif

/*
 * space needed for a null terminated string
 * of largest positive or negative number
 */
#ifndef msn_NUM_SIZE
#define msn_NUM_SIZE (20)
#endif

#endif _H_msncomp

