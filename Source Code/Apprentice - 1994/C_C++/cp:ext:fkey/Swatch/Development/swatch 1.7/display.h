/**

	display.h
	Copyright (c) 1990-1992, joe holt

 **/


#ifndef __display__
#define __display__

/*******************************************************************************
 **
 **	Headers
 **
 **/

#ifndef __ctypes__
#include "ctypes.h"
#endif
#ifndef __linked_list__
#include "linked_list.h"
#endif


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


#define MAX_COLUMNS				6

enum {
	colUpdating		= 1 << 0,
	colName			= 1 << 1,
	colSize			= 1 << 2,
	colFree			= 1 << 3,
	colIslands		= 1 << 4,
	colHeap			= 1 << 5
};


#define BORDER_TOP				0
#define BORDER_BOTTOM			16
#define CELL_HEIGHT				16


enum {
	UPDATE_UPDATING				= colUpdating,
	UPDATE_NAME					= colName,
	UPDATE_SIZE					= colSize,
	UPDATE_FREE					= colFree,
	UPDATE_ISLANDS				= colIslands,
	UPDATE_HEAP					= colHeap,
	UPDATE_SELECTION			= colHeap << 1,
	UPDATE_HEADER				= colHeap << 2,
	UPDATE_HEAP_SCALE			= colHeap << 3,
	UPDATE_BOTTOM				= colHeap << 4,
	FORCE_UPDATE				= 32768
};

enum {
	NO_UPDATE = 0,

	UPDATE_COLUMNS =			UPDATE_UPDATING |
								UPDATE_NAME |
								UPDATE_SIZE |
								UPDATE_FREE |
								UPDATE_ISLANDS |
								UPDATE_HEAP |
								UPDATE_SELECTION,

	UPDATE_IDLE_COLUMNS =		UPDATE_SIZE |
								UPDATE_FREE |
								UPDATE_ISLANDS |
								UPDATE_HEAP |
								UPDATE_SELECTION,

	UPDATE_COLUMNS_AND_BORDER =	UPDATE_COLUMNS |
								UPDATE_HEADER |
								UPDATE_HEAP_SCALE,

	UPDATE_EVERYTHING =			UPDATE_COLUMNS_AND_BORDER |
								UPDATE_BOTTOM
};

#define MBOX_EMPTY						0
#define MBOX_HEAP_ADDRESS				1
#define MBOX_ADDRESS					2

#define MBOX_ERASE						1
#define MBOX_FRAME_AND_CONTENTS			2
#define MBOX_CONTENTS					3

#define DISPLAY_ALL						-1

#define MIN_HEAP_SCALE			8
#define MAX_HEAP_SCALE			4194304


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

typedef struct {
	int16 left, right;
	struct _t_cell *cell;
} column_t;

typedef struct _t_cell {
	int16 type;
	ProcPtr draw;
	int16 width, space;
	int16 name;
} cell_t;

typedef struct {
	int16 top, bottom;
	int16 max_columns;
	column_t column[MAX_COLUMNS];
} row_t;

extern Boolean Use_color;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Display_init( void );
void Display_new_row_arrangement( int16 max_columns, int16 *column_types );
void Display_new_row( l_elem_t h );
void Display_update_row_tops( void );

int16 Display_row( int16 update, l_elem_t h );
void Display_lines( int16 update, int16 line_start, int16 line_end );

void Display_header( int16 update );
column_t *Display_find_column( struct _t_Heap_info *hi, int16 cell_type );

void Check_color_usage( void );
void Set_fore_color_or_pattern( int16 color );
void Set_fore_color( int16 color );
void Set_back_color( int16 color );
void Set_cursor( void );
unsigned char *MBox_text( void );

#endif
