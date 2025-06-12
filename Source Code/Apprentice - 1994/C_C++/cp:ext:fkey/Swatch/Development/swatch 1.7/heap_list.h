/**

	heap_list.h
	Copyright (c) 1990-1992, joe holt

 **/


#ifndef __heap_list__
#define __heap_list__

/*******************************************************************************
 **
 **	Headers
 **
 **/

#include <Processes.h>

#ifndef __ctypes__
#include "ctypes.h"
#endif
#ifndef __display__
#include "display.h"
#endif
#ifndef __linked_list__
#include "linked_list.h"
#endif


/*******************************************************************************
 **
 **	Public Constants
 **
 **/

enum {
	MAX_HEAP_TRANSITIONS = 5000
};

enum {
	BLOCK_TYPE_NOT_FREE_MASK = 0xC0,
	BLOCK_TYPE_NONRELOCATABLE_MASK = 0x40,
	BLOCK_TYPE_RELOCATABLE_MASK = 0x80,

	BLOCK_FLAG_RESOURCE_MASK = 0x20,
	BLOCK_FLAG_PURGE_MASK = 0x40,
	BLOCK_FLAG_LOCK_MASK = 0x80
};

/*******************************************************************************
 **
 **	Public Variables
 **
 **/

typedef struct {
	Ptr logical_start, logical_end;
	Ptr physical_start, physical_end;
	unsigned char type, flags;
	Handle handle;
} Heap_block_info_t;


typedef struct _t_Heap_info {
	THz zone;
	ProcessInfoRec info;
	struct _t_ExtendedProcessInfoRec **xinfo;
	unsigned char appname[32];

	Boolean touched:1;
	Boolean updating:1, last_updating:1;
	Boolean has_selection:1, last_has_selection:1;
	Boolean heap_is_32_bit:1;
	Boolean current_heap_ok:1, last_heap_ok:1;
	int16 selection_phase;
	row_t row;

	int32 current_size, last_size;
	int32 current_free, last_free;
	int16 current_islands, last_islands;
	int16 current_heap_transitions, last_heap_transitions;
	Heap_block_info_t current_selection, last_selection;
	int32 current_heap_offset_pixels, last_heap_offset_pixels;
	int32 current_heap[MAX_HEAP_TRANSITIONS], last_heap[MAX_HEAP_TRANSITIONS];
	int16 last_heap_x;
} Heap_info_t;

typedef l_elem_t Heap_info_handle_t;


extern Boolean Memory_Manager_is_32_bit;
extern l_anchor_t heaps;
extern Heap_info_handle_t multifinder, system;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Heap_list_init( void );
void Heap_list_exit( void );
int16 Heap_list_count(void);
int32 Heap_list_largest_heap_size( void );
Boolean Heap_list_update( void );
void Heap_list_update_one( Heap_info_handle_t h );
Boolean Heap_list_resort( void );

#endif
