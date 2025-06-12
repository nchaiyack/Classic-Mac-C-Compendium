/**

	swatch.h
	Copyright (c) 1990-1992, joe holt

 **/


#ifndef __swatch__
#define __swatch__

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
 **	Public Variables
 **
 **/

extern SysEnvRec This_mac;
extern Boolean in_foreground;
extern Boolean Debugger_installed;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

#if 1
#define assert(expression, message) \
	( (expression) ? (void) 0 : DebugStr(message) )
#else
#define assert(ignore1, ignore2) ((void) 0)
#endif

Boolean Swatch_in_foreground( void );
void Bail( int16 alert_number );
unsigned char *pstr( int16 index );

#endif  /* ifndef _H_SWATCH */
