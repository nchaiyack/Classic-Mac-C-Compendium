/**

	heap.h
	Copyright (c) 1990-1992, joe holt

 **/


#ifndef __heap__
#define __heap__

/*******************************************************************************
 **
 **	Headers
 **
 **/

#ifndef __heap_list__
#include "heap_list.h"
#endif


/*******************************************************************************
 **
 **	Public Constants
 **
 **/

// ordered in increasing display priority

enum {
	HEAP_FREE_RUN			= 1 << 0,
	HEAP_PURGEABLE_RUN		= 1 << 1,
	HEAP_UNLOCKED_RUN		= 1 << 2,
	HEAP_LOCKED_RUN			= 1 << 3
};


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Heap_update_current( Heap_info_handle_t h );
Boolean Heap_locate_block( Heap_info_t *d, void *address, Heap_block_info_t *info );
Boolean get_x_process_information( Heap_info_t *d );

#endif
