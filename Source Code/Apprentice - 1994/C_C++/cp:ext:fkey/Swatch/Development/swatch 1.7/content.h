/**

	content.h
	Copyright (c) 1990-1992, joe holt

 **/


#ifndef __content__
#define __content__

/*******************************************************************************
 **
 **	Headers
 **
 **/

#ifndef __ctypes__
#include "ctypes.h"
#endif
#ifndef __heap_list__
#include "heap_list.h"
#endif


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Content_init( void );
void Content_mousedown( EventRecord *the_event );
void Content_set_cursor( EventRecord *the_event );
int16 Content_rows( void );
void Content_reset_vert( void );
void Content_horiz_scrolled( int cvalue );
Boolean Content_reset_horiz( ControlHandle the_control );
int16 Content_partial_row( void );
void Content_scroll_horiz( ControlHandle the_control, int16 part, int16 unit_delta );

int32 Content_heap_offset_pixels( void );

void *Content_pixel_to_address( Heap_info_t *d, int32 pixel_h );
int32 Content_address_to_pixel( Heap_info_t *d, void *address );
Boolean Content_find_point( Point where, Heap_info_handle_t *hp, int16 *type );

void record_for_mbox( Heap_info_t *hi );

#endif
