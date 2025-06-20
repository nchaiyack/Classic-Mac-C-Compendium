/**

	swatch.prefs.h
	Copyright (c) 1990, Adobe Systems, Inc.

 **/


#ifndef _H_SWATCH_PREFS
#define _H_SWATCH_PREFS

#include <ctypes.h>

/*******************************************************************************
 **
 **	Public Constants
 **
 **/

#define PREFS_VERSION	1

#define CELL_HEIGHT			16
#define NAME_TOP_DELTA		2
#define SIZE_TOP_DELTA		2
#define FREE_TOP_DELTA		2
#define TEXT_HEIGHT			12

#define NAME_FIELD_LEFT		5
#define NAME_FIELD_RIGHT	85
#define SIZE_FIELD_LEFT		90
#define SIZE_FIELD_RIGHT	150
#define FREE_FIELD_LEFT		155
#define FREE_FIELD_RIGHT	210
#define HEAP_FIELD_LEFT		220
#define HEAP_FIELD_RIGHT	16384

#define DEFAULT_HEAP_SCALE		8192
#define DEFAULT_HEAP_SCALE_2N	13

#define DEFAULT_WINDOW_LEFT		50
#define DEFAULT_WINDOW_TOP		50
#define DEFAULT_WINDOW_RIGHT	408
#define DEFAULT_WINDOW_BOTTOM	(DEFAULT_WINDOW_TOP + 10 * CELL_HEIGHT)


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

typedef struct {
	int16 version;
	Rect window_rect;
	int32 heap_scale;
	int16 heap_scale_2n;
} Preferences_t;

extern Preferences_t Swatch_prefs;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Read_prefs( void );
void Write_prefs( void );
Handle Read_riff( void );
void Dispose_riff( Handle riff );


#endif  /* ifndef _H_SWATCH_PREFS */
