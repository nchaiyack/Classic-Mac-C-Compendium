/**

	pstring.h
	Copyright (c) 1990-1992, joe holt

 **/


#ifndef __pstring__
#define __pstring__

/*******************************************************************************
 **
 **	Headers
 **
 **/

#ifndef __ctypes__
#include "ctypes.h"
#endif


/*******************************************************************************
 **
 **	Public Macros
 **
 **/

#define pstrlen(s1)		( *(unsigned char *)s1 )


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

unsigned char *pstrcopy( unsigned char *s1, unsigned char *s2 );
unsigned char *pstrappend( unsigned char *s1, unsigned char *s2 );
unsigned char *pchappend( unsigned char ch, unsigned char *s2 );
unsigned char *pchcopy( unsigned char ch, unsigned char *s2 );
unsigned char *pnumcopy( int32 n, unsigned char *s2 );
unsigned char *pnumappend( int32 n, unsigned char *s2 );
unsigned char *phexcopy( uns32 n, int16 d, unsigned char *s2 );
unsigned char *phexappend( uns32 n, int16 d, unsigned char *s2 );
Boolean pstrequal( unsigned char *s1, unsigned char *s2 );
unsigned char chtoupper( unsigned char ch );
void blockzero( unsigned char *p1, int32 len );

#endif  /* ifndef __pstring__ */
