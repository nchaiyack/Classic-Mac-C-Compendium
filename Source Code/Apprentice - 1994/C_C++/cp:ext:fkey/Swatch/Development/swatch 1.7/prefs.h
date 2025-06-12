/**

	prefs.h
	Copyright (c) 1990-1992, joe holt

 **/


#ifndef __prefs__
#define __prefs__

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

typedef struct {
	int16 version;
	Boolean dirty;
	Rect window_rect;
	int32 heap_scale;
	int16 heap_scale_2n;
	int16 wne_in_foreground, wne_in_background;
} Preferences_t;

extern Preferences_t Prefs;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Prefs_init( void );
void Prefs_save( void );

#endif
