/**

	drvr.csOpen.c
	Copyright (c) 1990, joe holt

 **/


#ifndef _H_DRVR_CSOPEN
#define _H_DRVR_CSOPEN


/**-----------------------------------------------------------------------------
 **
 ** Headers
 **
 **/

#include <types.h>


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

extern SysEnvRec The_world;
extern int16 INIT_RefNum;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

OSErr csOpen( CntrlParam *cpb, DCtlPtr dce );


#endif  /* ifndef _H_DRVR_CSOPEN */
