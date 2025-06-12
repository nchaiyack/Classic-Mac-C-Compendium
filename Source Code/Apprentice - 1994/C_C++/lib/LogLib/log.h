/*
	log.h		Header file for log.c
	
	
		©1994 by Dave Nebinger (dnebing@andy.bgsu.edu)
		All Rights Reserved.
		
		Feel free to use this where ever you wish, just drop me some email
		stating what you are doing with it.
*/

#pragma once

#ifndef __H_log__
#define __H_log__

/*
	Apple includes
*/
#include <Script.h>	/* for smSystemScript */
#include <Folders.h>

/*
	ANSI-small includes
	
	These allow for the use of the '...' and also some of the string and printing
	functions are used.
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/*
	First a constant
*/
#define kNewLine 0x0d /* use this constant instead of \n. */


/*
	Now for those nasty prototypes...
*/
Boolean InitLog(Str255 fold,short* vref,long* pid,long* fid);
short OpenLog(Str255 name,short vref,long did);
Boolean Log(short,const char* fmt,...);
Boolean LogTime(short,const char* fmt,...);
void CloseLog(short);

#endif /* __H_log__ */
