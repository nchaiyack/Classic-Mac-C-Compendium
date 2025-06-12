/**

	drvr.csControl.c
	Copyright (c) 1990, joe holt

 **/


#ifndef _H_DRVR_CSCONTROL
#define _H_DRVR_CSCONTROL


/*******************************************************************************
 **
 **	Public Constants
 **
 **/

enum {
	gVersion = 1000
};


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

OSErr csControl( CntrlParam *cpb, DCtlPtr dce );


#endif  /* ifndef _H_DRVR_CSCONTROL */
