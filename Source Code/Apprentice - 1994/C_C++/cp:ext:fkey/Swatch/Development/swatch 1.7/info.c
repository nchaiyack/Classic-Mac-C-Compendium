/**

	info.c
	Copyright (c) 1990-1992, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#ifndef __content__
#include "content.h"
#endif
#ifndef __ctypes__
#include "ctypes.h"
#endif
#ifndef __display__
#include "display.h"
#endif
#ifndef __heap__
#include "heap.h"
#endif
#ifndef __heap_list__
#include "heap_list.h"
#endif
#ifndef __info__
#include "info.h"
#endif
#ifndef __prefs__
#include "prefs.h"
#endif
#ifndef __pstring__
#include "pstring.h"
#endif
#ifndef __resources__
#include "resources.h"
#endif
#ifndef __swatch__
#include "swatch.h"
#endif
#ifndef __window__
#include "window.h"
#endif


/**-----------------------------------------------------------------------------
 **
 ** Private Constants
 **
 **/

// resources describing which low memory globals are saved

#define OLD_lmem	-16459
#define NEW_lmem	-16458


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

typedef struct {
	int16 run_length;
	Ptr start_address;
} lmem_elem_t;

static lmem_elem_t *lmem_ptr;
static Ptr low_memory;
static Handle top_map_hdl;

extern int16 mbox_type;
extern Heap_info_t *mbox_heap;


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

static Boolean set_up_process_access( Heap_info_t *d );
static Boolean set_up_process_low_memory_access( Heap_info_t *d );
static int32 get_process_low_memory_value( void *address, int16 size );
static void identify_resource( Handle map, Handle h, unsigned char *mbox_text );
static void identify_block( Heap_info_handle_t h, Heap_info_t *hi, unsigned char *mbox_text );


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

/*******************************************************************************
 **
 **	Public Functions
 **
 **/

// bug in 1.7d5--ants don't display while tracking if vert scroll not at top

void Info_track_heap( EventRecord *the_event, Heap_info_handle_t h )
{
	Heap_info_handle_t new_h;
	Heap_info_t *hi;
	unsigned char *mbox_text;
	int32 current_address;
	Boolean current_found, mbox_needs_updating;
	uns32 ticker;

	mbox_text = MBox_text();
	mbox_needs_updating = false;


	// find heap with selection, if there is one

	{
		Boolean has_selection;

		h = nil;
		while ( (h = l_next( heaps, h )) ) {
			Heap_info_t *hi = l_access( h );
			has_selection =  hi->has_selection;
			l_release( h );
			if ( has_selection ) break;
		}
	}


	while ( StillDown() ) {

		GetMouse( &the_event->where );


		{
			Heap_info_handle_t temp_h;
			Heap_info_t *hi_temp;
			column_t *col;
			int16 right;

			temp_h = l_next( heaps, nil );
			hi_temp = l_access( temp_h );
			col = Display_find_column( hi_temp, colHeap );

			right = App_window->portRect.right - 16;
				
			if ( the_event->where.h >= right ) {
				Content_scroll_horiz( horiz_scroll, inDownButton, 10 );
				the_event->where.h = right;
			}
			else if ( the_event->where.h <= col->left ) {
				Content_scroll_horiz( horiz_scroll, inUpButton, 10 );
				the_event->where.h = col->left;
			}
	
			l_release( temp_h );
		}


		if ( !Content_find_point( the_event->where, &new_h, nil ) ) new_h = nil;

		if ( new_h != h ) {
			if ( h ) {

				hi = l_access( h );
				hi->has_selection = false;
				Display_row( UPDATE_SELECTION, h );  // remove old selection
				l_release( h );

			}
			h = new_h;
		}


		if ( h ) {

			hi = l_access( h );

			if ( !(TickCount() % 3) )
				hi->selection_phase = (hi->selection_phase + 1) & 7;

			current_address = (int32) Content_pixel_to_address( hi, the_event->where.h );
			hi->has_selection = Heap_locate_block( hi, (Ptr)current_address,
					&hi->current_selection );

			if ( hi->has_selection ) {

				if ( hi->has_selection != hi->last_has_selection ||
					(hi->current_selection.physical_start != hi->last_selection.physical_start ||
					 hi->current_selection.physical_end != hi->last_selection.physical_end) ) {

					mbox_needs_updating = true;
					mbox_type = MBOX_ADDRESS;
					phexcopy( (int32) hi->current_selection.logical_start, 8, mbox_text );
					pstrappend( "\p #", mbox_text );
					pnumappend( (int32) (hi->current_selection.logical_end -
							hi->current_selection.logical_start) + 1, mbox_text );
					pchappend( ' ', mbox_text );
					identify_block( h, hi, mbox_text );

					hi->has_selection = true;
					record_for_mbox( hi );

				}

			}
			else if ( mbox_type != MBOX_EMPTY ) {

				mbox_needs_updating = true;
				mbox_text[0] = 0;
				mbox_type = MBOX_EMPTY;

			}

			Display_row( UPDATE_SELECTION, h );

			l_release( h );

		}
		else if ( mbox_type != MBOX_EMPTY ) {

			mbox_needs_updating = true;
			mbox_text[0] = 0;
			mbox_type = MBOX_EMPTY;

		}


		if ( mbox_needs_updating ) {
			mbox_needs_updating = false;
			Window_update_mbox();
		}



	for ( ticker = TickCount() + 1; ticker > TickCount(); );
	}
}


static Boolean set_up_process_access( Heap_info_t *d )
{
	if ( !set_up_process_low_memory_access( d ) ) {
		//DebugStr( "\pset_up_process_low_memory_access() failed." );
		return false;
	}
	top_map_hdl = (Handle) get_process_low_memory_value( &TopMapHndl, 4 );
	return true;
}


static Boolean set_up_process_low_memory_access( Heap_info_t *d )
{
	Handle lmem_hdl;

	lmem_ptr = nil;
	low_memory = nil;

	lmem_hdl = GetResource( 'lmem', This_mac.hasColorQD ? NEW_lmem : OLD_lmem );
	if ( !lmem_hdl ) return false;
	lmem_ptr = *(lmem_elem_t **)lmem_hdl;	// assume 'lmem' resource is already locked

	if ( !get_x_process_information( d ) ) return false;
	low_memory = *(**d->xinfo).processLowMemory;	// don't lock, so use is limited
	return true;
}


static int32 get_process_low_memory_value( void *address, int16 size )
{
	lmem_elem_t *l;
	Ptr value_address, start_address;
	int16 run_length;
	int32 value;

	if ( !low_memory ) return 0;

	l = lmem_ptr;
	value_address = low_memory;
	for ( ; (run_length = l->run_length); value_address += run_length, ++l ) {
		start_address = l->start_address;
		if ( (Ptr)address < (start_address + run_length) && (Ptr)address >= start_address ) {
			value_address += (Ptr)address - start_address;
			goto got_it;
		}
	}
	value_address = (Ptr)address;	// not a saved value, so use real low memory global
got_it:
	for ( value = 0; size; --size ) value = (value << 8) | *(unsigned char *)value_address++;
	return value;
}


typedef struct _t_resource_map {
	char who_cares[16];
	struct _t_resource_map **next_map_hdl;
	int16 ref_num;
	int16 attributes;
	int16 type_list_offset;
	int16 name_list_offset;
} resource_map_t;

typedef struct {
	int32 type;
	int16 count;
	int16 reference_list_offset;
} resource_type_t;

typedef struct {
	int16 id;
	int16 name_offset;
	int32 data_offset;
	Handle handle;
} resource_reference_t;


static void identify_resource( Handle map, Handle h, unsigned char *mbox_text )
{
	resource_map_t *m;
	resource_type_t *t;
	resource_reference_t *r;
	Ptr base_t;
	int16 type_count, reference_count;

	m = *(resource_map_t **)map;
	for ( ;; ) {
		base_t = (Ptr)((int32)m + m->type_list_offset);
		type_count = *(int16 *)base_t;
		for ( t = (resource_type_t *)(base_t + 2); type_count >= 0; --type_count, ++t ) {
			reference_count = t->count;
			r = (resource_reference_t *)((int32)base_t + t->reference_list_offset);
			for ( ; reference_count >= 0; --reference_count, ++r ) {
				if ( r->handle == h ) {

					pchappend( ' ', mbox_text );
					pchappend( t->type >> 24, mbox_text );
					pchappend( t->type >> 16, mbox_text );
					pchappend( t->type >> 8, mbox_text );
					pchappend( t->type, mbox_text );
					pchappend( ' ', mbox_text );
					pnumappend( r->id, mbox_text );
//					pstrappend( "\p $", mbox_text );
//					phexappend( m->ref_num, 4, mbox_text );
					if ( r->name_offset != -1 ) {
						pstrappend( "\p '", mbox_text );
						pstrappend( (unsigned char *)((int32)m + m->name_list_offset + r->name_offset),
								mbox_text );
						pchappend( '\'', mbox_text );
					}
					return;

				}
			}
		}
		if ( !m->next_map_hdl ) break;
		m = *m->next_map_hdl;
	}

	pstrappend( "\punidentifiable resource", mbox_text );
}


static void identify_block( Heap_info_handle_t h, Heap_info_t *hi, unsigned char *mbox_text )
{
	if ( !(hi->current_selection.type & BLOCK_TYPE_NOT_FREE_MASK) ) {	// free block
		pstrappend( "\pfree", mbox_text );
	}


	else if ( h == multifinder ) {
		pstrappend( "\pmf block", mbox_text );
		// identify multifinder apps and blocks allocated by apps
	}


	else if ( h == system ) {
		if ( (hi->current_selection.flags & BLOCK_FLAG_RESOURCE_MASK) )
			identify_resource( SysMapHndl, hi->current_selection.handle, mbox_text );
		else {
			// identify system block
			pstrappend( "\psystem block", mbox_text );
		}
	}


	else if ( hi->info.processNumber.highLongOfPSN ||
			hi->info.processNumber.lowLongOfPSN != kNoProcess ) {

	// $01234567 CODE 2 'Main'

		if ( set_up_process_access( hi ) ) {

			if ( (hi->current_selection.flags & BLOCK_FLAG_RESOURCE_MASK) )
				identify_resource( top_map_hdl, hi->current_selection.handle, mbox_text );
			else {
				// identify app block
				pstrappend( "\papp block", mbox_text );
			}
		}
	}
}
