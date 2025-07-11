/**

	swatch.h
	Copyright (c) 1990, Adobe Systems, Inc.

 **/


#ifndef _H_SWATCH
#define _H_SWATCH

#include <ctypes.h>

/*******************************************************************************
 **
 **	Public Macros
 **
 **/


/*******************************************************************************
 **
 **	Public Constants
 **
 **/

#define BLACK_COLOR				0
#define WHITE_COLOR				1
#define HEADER_COLOR			2
#define HEADER_BORDER_COLOR		3
#define NAME_COLOR				4
#define SIZE_COLOR				5
#define HEAP_BORDER_COLOR		6
#define HEAP_FREE_COLOR			7
#define HEAP_LOCKED_COLOR		8
#define HEAP_UNLOCKED_COLOR		9
#define FREE_COLOR				10
#define BACK_COLOR				11
#define HEAP_PURGEABLE_COLOR	12


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

extern SysEnvRec This_mac;
extern Boolean Use_color;
extern Boolean Pre_system_7;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void set_fore_color_or_pattern( int16 color );
void set_fore_color( int16 color );
void set_back_color( int16 color );

#endif  /* ifndef _H_SWATCH */
