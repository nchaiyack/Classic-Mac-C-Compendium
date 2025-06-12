/**

	drvr.globals.h
	Copyright (c) 1990, Adobe Systems, Inc.

 **/


#ifndef _H_DRVR_GLOBALS
#define _H_DRVR_GLOBALS

#include <ctypes.h>

/*******************************************************************************
 **
 **	Public Constants
 **
 **/

#define VERSION		12

#define MAX_APPS	15

enum {
	csGetGlobals = 1000
};


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

typedef struct {
	Boolean free;
	THz zone;
	char appname[32];
} App_stat_t;


typedef struct {
	int16 max_apps;
	int16 zero;
	int16 version;
	int16 num_apps;
	App_stat_t *apps;
	Boolean apps_need_updating;
} Swatch_globals_t;

extern Swatch_globals_t Globals;


#endif  /* ifndef _H_DRVR_GLOBALS */
