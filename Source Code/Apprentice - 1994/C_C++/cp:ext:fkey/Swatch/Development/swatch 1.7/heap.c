/**

	heap.c
	Copyright (c) 1990-1992, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#ifndef __ctypes__
#include "ctypes.h"
#endif
#ifndef __heap__
#include "heap.h"
#endif
#ifndef __info__
#include "info.h"
#endif
#ifndef __prefs__
#include "prefs.h"
#endif
#ifndef __swatch__
#include "swatch.h"
#endif


/**-----------------------------------------------------------------------------
 **
 ** Private Constants
 **
 **/

#define BLOCK_TYPE_MASK					0xC0
#define BLOCK_FREE						0x00
#define BLOCK_NON_RELOCATABLE			0x40
#define BLOCK_RELOCATABLE				0x80

#define MASTER_POINTER_LOCK_MASK		0x80
#define MASTER_POINTER_PURGE_MASK		0x40
#define MASTER_POINTER_RESOURCE_MASK	0x20


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

/*

	A 32-bit block header is:

	1 byte block type ($40 = non-relo, etc)
	1 byte block flags ($80 = locked, etc)
	2 bytes alignment
	1 long size of block
	1 long relative handle, etc

	The physical size of the block is in the second long word. To get to the next block
	header, just add the size of the block.

*/

typedef struct {
	unsigned char type;
	unsigned char flags;
	int16 alignment;
	int32 size;
	int32 relative_handle;
} header_32_t;


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

static void make_24_bit_heap( Heap_info_t *d );
static void make_32_bit_heap( Heap_info_t *hi );
static void make_32_bit_heap_old( Heap_info_t *d );


/*******************************************************************************
 **
 **	Public Functions
 **
 **/


void Heap_update_current( Heap_info_handle_t h )
{
	Heap_info_t *hi = l_access( h );
	if ( hi->heap_is_32_bit )
		make_32_bit_heap( hi );
	else
		make_24_bit_heap( hi );
	l_release( h );
}


Boolean Heap_locate_block( Heap_info_t *d, void *address, Heap_block_info_t *info )
{
	int32 blocks_checked;

	blocks_checked = 0;

	if ( d->heap_is_32_bit ) {
		Ptr bh, bkLim;
		int32 size;
	
		bh = (Ptr) &d->zone->heapData;
		bkLim = d->zone->bkLim;

		for ( ;; ) {
			++blocks_checked;

			if ( bh > bkLim || ( (int32) bh & 1 ) )
				return false;		// heap bad

			if ( bh == bkLim )
				return false;		// block not in this heap

			size = ((header_32_t *)bh)->size;
			if ( bh + size <= bh )
				return false;		// heap bad

			if ( (Ptr)address < (bh + size) && (Ptr)address >= bh ) {
				info->physical_start = bh;
				info->physical_end = bh + size - 1;
				info->logical_start = bh + 12;
				info->logical_end = info->physical_end - ((header_32_t *)bh)->alignment;
				info->type = ((header_32_t *)bh)->type & 0xC0;
				info->flags = ((header_32_t *)bh)->flags & 0xE0;
				if ( info->type & BLOCK_TYPE_RELOCATABLE_MASK )
					info->handle =
							(Handle)(((header_32_t *)bh)->relative_handle + (int32)d->zone);
				else info->handle = NULL;
				return true;
			}

			bh += size;
		}
	}
	else {
		Ptr bh, bkLim;
		int32 size;
	
		bh = (Ptr) &d->zone->heapData;
		bkLim = d->zone->bkLim;

		for ( ;; ) {
			++blocks_checked;

			if ( bh > bkLim || ( (int32) bh & 1 ) )
				return false;		// heap bad

			if ( bh == bkLim )
				return false;		// block not in this heap

			size = *(int32 *) bh & 0x00FFFFFF;
			if ( bh + size <= bh )
				return false;		// heap bad

			if ( (Ptr)address < (bh + size) && (Ptr)address >= bh ) {
				info->physical_start = bh;
				info->physical_end = bh + size - 1;
				info->logical_start = bh + 8;
				info->logical_end = info->physical_end - (*(unsigned char *)bh & 0x0F);
				info->type = *(unsigned char *)bh & 0xC0;
				if ( info->type & BLOCK_TYPE_RELOCATABLE_MASK                                                                                                                                                                                                                                                                                                                                                                                                                                                               ) {
					info->handle =
							(Handle)(*(int32 *)(bh + 4) + (int32)d->zone);
					info->flags = *(unsigned char *)info->handle & 0xE0;
				}
				else {
					info->handle = NULL;
					info->flags = 0;
				}
				return true;
			}

			bh += size;
		}
	}
}


Boolean get_x_process_information( Heap_info_t *d )
{
	THz zone;
	Ptr bh, bkLim;
	int32 size;
	Boolean is_32;
	Heap_info_t *hi;

	if ( d->xinfo ) return true;

// first time thru -- gotta look for it

	hi = l_access( multifinder );
	zone = hi->zone;
	bh = (Ptr) &zone->heapData;
	bkLim = zone->bkLim;
	is_32 = hi->heap_is_32_bit;
	l_release( multifinder );

	if ( is_32 ) {

		for ( ;; ) {

			if ( bh > bkLim || ( (int32) bh & 1 ) ) return false;  // heap bad

			if ( bh == bkLim ) return false;  // block not in this heap

			size = ((header_32_t *)bh)->size;
			if ( bh + size <= bh ) return false;  // heap bad

			if ( size >= sizeof(ExtendedProcessInfoRec) &&
				((header_32_t *)bh)->type == BLOCK_RELOCATABLE ) {
				ExtendedProcessInfoRec *p = (ExtendedProcessInfoRec *) (bh + 12);
				if ( p->processSignature == d->info.processSignature &&
					 p->processType == d->info.processType ) {
					d->xinfo = (ExtendedProcessInfoRec **)
						((int32) zone + ((header_32_t *)bh)->relative_handle);
					return true;
				}
			}

			bh += size;

		}

	}
	else {

		for ( ;; ) {

			if ( bh > bkLim || ( (int32) bh & 1 ) ) return false;  // heap bad

			if ( bh == bkLim ) return false;  // block not in this heap

			size = *(int32 *) bh & 0x00FFFFFF;
			if ( bh + size <= bh ) return false;  // heap bad

			if ( size >= sizeof(ExtendedProcessInfoRec) &&
				(*(unsigned char *) bh & BLOCK_TYPE_MASK) == BLOCK_RELOCATABLE ) {
				ExtendedProcessInfoRec *p = (ExtendedProcessInfoRec *) (bh + 8);
				if ( p->processSignature == d->info.processSignature &&
					 p->processType == d->info.processType ) {
					d->xinfo = (ExtendedProcessInfoRec **)
						((int32) zone + (*(int32 *) (bh + 4)));
					return true;
				}
			}

			bh += size;
		}
	}

	return false;
}


static void make_24_bit_heap( Heap_info_t *d )
{
	Ptr bh, tp;
	THz zone;
	int32 chunk_size, size;
	uns16 run_type, block_type;
	uns16 last_run_type;
	int32 this_run;
	int32 *p;
	int32 i;

	zone = d->zone;
	bh = (Ptr) &zone->heapData;

	d->current_heap_transitions = 1;
	p = &d->current_heap[0];
	run_type = last_run_type = HEAP_LOCKED_RUN;
	this_run = (int32) run_type << 24;
	chunk_size = 0;

	for (;;) {
		if ( bh >= BufPtr || ( (int32) bh & 1 ) )
			goto heap_bad;

		size = *(int32 *) bh & 0x00FFFFFF;
		block_type = *(unsigned char *) bh & BLOCK_TYPE_MASK;
		if ( (chunk_size += size) >= Prefs.heap_scale ) {
			if ( run_type == last_run_type )
				++this_run;
			else {
				*p++ = this_run;
				if ( d->current_heap_transitions == MAX_HEAP_TRANSITIONS )
					break;
				++d->current_heap_transitions;
				this_run = ((int32) run_type << 24) + 1;
				last_run_type = run_type;
			}

			if ( !block_type )
				run_type = HEAP_FREE_RUN;
			else if ( block_type == BLOCK_NON_RELOCATABLE )
				run_type = HEAP_LOCKED_RUN;
			else {
				tp = (Ptr) zone + *(int32 *) (bh + 4);
				if ( tp >= BufPtr || ( (int32) tp & 1 ) )
					goto heap_bad;
				if ( *tp & MASTER_POINTER_LOCK_MASK )
					run_type = HEAP_LOCKED_RUN;
				else if ( *tp & MASTER_POINTER_PURGE_MASK )
					run_type = HEAP_PURGEABLE_RUN;
				else
					run_type = HEAP_UNLOCKED_RUN;
			}
			chunk_size -= Prefs.heap_scale;
			if ( chunk_size >= Prefs.heap_scale ) {
				if ( run_type != last_run_type ) {
					*p++ = this_run;
					if ( d->current_heap_transitions == MAX_HEAP_TRANSITIONS )
						break;
					++d->current_heap_transitions;
					this_run = (int32) run_type << 24;
					last_run_type = run_type;
				}
				i = chunk_size >> Prefs.heap_scale_2n;
				this_run += i;
				chunk_size -= (int32) i << Prefs.heap_scale_2n;
			}
		}
		else if ( run_type != HEAP_LOCKED_RUN ) {
			if ( !block_type )
				;
			else if ( block_type == BLOCK_NON_RELOCATABLE )
				run_type = HEAP_LOCKED_RUN;
			else {
				tp = (Ptr) zone + *(int32 *) (bh + 4);
				if ( tp >= BufPtr || ( (int32) tp & 1 ) )
					goto heap_bad;
				if ( *tp & MASTER_POINTER_LOCK_MASK )
					run_type = HEAP_LOCKED_RUN;
				else if ( *tp & MASTER_POINTER_PURGE_MASK )
					run_type = HEAP_PURGEABLE_RUN;
				else
					run_type = HEAP_UNLOCKED_RUN;
			}
		}

		if ( bh + size <= bh )
			goto heap_bad;
		bh += size;

		if ( bh < zone->bkLim )
			continue;

		if ( bh > zone->bkLim )
			goto heap_bad;

		if ( chunk_size )
			++this_run;
		*p = this_run;
		break;
	}

	d->current_heap_ok = TRUE;
	return;

heap_bad:
	d->current_heap_ok = FALSE;
}


static uns8 run_type_priority[] = {  // zero element not used
	1, 1, 2, 2, 4, 4, 4, 4,
	8, 8, 8, 8, 8, 8, 8, 8
};


static void make_32_bit_heap( Heap_info_t *hi )
{
	Ptr block;
	THz zone;
	int32 bytes_this_pixel, block_size;
	uns16 block_type;
	int32 pixels_this_run, *p;

	int32 bytes_per_pixel;
	int16 bytes_per_pixel_2n;

	uns8 run_type, display_type, last_display_type, display_types_seen;

	zone = hi->zone;
	block = (Ptr) &zone->heapData;

	bytes_per_pixel = Prefs.heap_scale;
	bytes_per_pixel_2n = Prefs.heap_scale_2n;

	hi->current_islands = 0;

	hi->current_heap_transitions = 0;
	p = &hi->current_heap[0];

	pixels_this_run = 0;
	bytes_this_pixel = 0;
	display_types_seen = 0;
	last_display_type = HEAP_LOCKED_RUN;		// assume heap begins with locked blocks
	run_type = HEAP_LOCKED_RUN;


	while ( block < zone->bkLim ) {


// heap okay check

		if ( block >= BufPtr || ( (int32) block & 1 ) ) goto heap_bad;

		block_size = ((header_32_t *) block)->size;


// classify block type: free, purgeable, unlocked or locked

		block_type = ((header_32_t *) block)->type;
		if ( block_type == BLOCK_FREE )
			display_type = HEAP_FREE_RUN;
		else if ( block_type == BLOCK_NON_RELOCATABLE )
			display_type = HEAP_LOCKED_RUN;
		else {
			Ptr tp = (Ptr) zone + ((header_32_t *) block)->relative_handle;
			if ( tp >= BufPtr || ( (int32) tp & 1 ) )
				goto heap_bad;
			if ( ((header_32_t *) block)->flags & MASTER_POINTER_LOCK_MASK )
				display_type = HEAP_LOCKED_RUN;
			else if ( ((header_32_t *) block)->flags & MASTER_POINTER_PURGE_MASK )
				display_type = HEAP_PURGEABLE_RUN;
			else
				display_type = HEAP_UNLOCKED_RUN;
		}


// include this block's type in list of types seen for this pixel

		display_types_seen |= display_type;


// update island count at block resolution (not pixel resolution)

		if ( display_type != last_display_type ) {
			last_display_type = display_type;
			if ( display_type == HEAP_LOCKED_RUN ) ++hi->current_islands;
		}


		bytes_this_pixel += block_size;
		if ( bytes_this_pixel >= bytes_per_pixel ) {

			uns8 this_pixel_type = run_type_priority[display_types_seen];
			bytes_this_pixel -= bytes_per_pixel;

			if ( this_pixel_type == run_type ) ++pixels_this_run;
			else {

				*p++ = ((int32)run_type << 24) | pixels_this_run;
				++hi->current_heap_transitions;

				if ( hi->current_heap_transitions == MAX_HEAP_TRANSITIONS )
					goto heap_too_messy;

				run_type = this_pixel_type;
				pixels_this_run = 1;

			}


			if ( bytes_this_pixel >= bytes_per_pixel ) {

				uns8 addl_pixels_type = run_type_priority[display_type];
				int32 addl_pixels = bytes_this_pixel >> bytes_per_pixel_2n;
				bytes_this_pixel -= addl_pixels << bytes_per_pixel_2n;

				if ( addl_pixels_type == run_type ) pixels_this_run += addl_pixels;
				else {

					*p++ = ((int32)run_type << 24) | pixels_this_run;
					++hi->current_heap_transitions;
	
					if ( hi->current_heap_transitions == MAX_HEAP_TRANSITIONS )
						goto heap_too_messy;
	
					run_type = addl_pixels_type;
					pixels_this_run = addl_pixels;

				}
	
			}

			display_types_seen = (bytes_this_pixel) ? display_type : 0;

		}


// skip to next block; heap okay checks

		if ( block + block_size <= block ) goto heap_bad;
		block += block_size;

	}


// at this point we've checked every block in the heap

	if ( block > zone->bkLim ) goto heap_bad;


// include last partial pixel

	if ( bytes_this_pixel ) {

		uns8 final_pixel_type = run_type_priority[display_types_seen];

		if ( final_pixel_type == run_type ) ++pixels_this_run;
		else {

			*p++ = ((int32)run_type << 24) | pixels_this_run;
			++hi->current_heap_transitions;

			if ( hi->current_heap_transitions == MAX_HEAP_TRANSITIONS )
				goto heap_too_messy;

			run_type = final_pixel_type;
			pixels_this_run = 1;

		}

	}


// include last run

	*p++ = ((int32)run_type << 24) | pixels_this_run;
	++hi->current_heap_transitions;


// last locked block at end of heap is not an island

	if ( display_type == HEAP_LOCKED_RUN && hi->current_islands > 0 )
		--hi->current_islands;


	hi->current_heap_ok = TRUE;
	return;

heap_bad:
	hi->current_heap_ok = FALSE;
	return;

heap_too_messy:
	DebugStr( "\pHeap too messy...setting heap status to bad, but it isn't, really" );
	hi->current_heap_ok = FALSE;	
}


#if 0
static void make_32_bit_heap_old( Heap_info_t *d )
{
	Ptr tp, bh;
	THz zone;
	int32 chunk_size, size;
	uns16 run_type, block_type;
	uns16 last_run_type;
	int32 this_run;
	int32 *p;
	int32 i;
	int32 heap_scale;
	int16 heap_scale_2n;

	zone = d->zone;
	bh = (Ptr) &zone->heapData;

	d->current_heap_transitions = 1;
	p = &d->current_heap[0];
	run_type = last_run_type = HEAP_LOCKED_RUN;
	this_run = (int32) run_type << 24;
	chunk_size = 0;

	heap_scale = Prefs.heap_scale;
	heap_scale_2n = Prefs.heap_scale_2n;

	for (;;) {
		if ( bh >= BufPtr || ( (int32) bh & 1 ) )
			goto heap_bad;

		size = ((header_32_t *) bh)->size;
		block_type = ((header_32_t *) bh)->type;

		if ( (chunk_size += size) >= heap_scale ) {
			if ( run_type == last_run_type )
				++this_run;
			else {
				*p++ = this_run;
				if ( d->current_heap_transitions == MAX_HEAP_TRANSITIONS )
					break;
				++d->current_heap_transitions;
				this_run = ((int32) run_type << 24) + 1;
				last_run_type = run_type;
			}

			if ( block_type == BLOCK_FREE )
				run_type = HEAP_FREE_RUN;
			else if ( block_type == BLOCK_NON_RELOCATABLE )
				run_type = HEAP_LOCKED_RUN;
			else {
				tp = (Ptr) zone + ((header_32_t *) bh)->relative_handle;
				if ( tp >= BufPtr || ( (int32) tp & 1 ) )
					goto heap_bad;
				if ( ((header_32_t *) bh)->flags & MASTER_POINTER_LOCK_MASK )
					run_type = HEAP_LOCKED_RUN;
				else if ( ((header_32_t *) bh)->flags & MASTER_POINTER_PURGE_MASK )
					run_type = HEAP_PURGEABLE_RUN;
				else
					run_type = HEAP_UNLOCKED_RUN;
			}
			chunk_size -= heap_scale;
			if ( chunk_size >= heap_scale ) {
				if ( run_type != last_run_type ) {
					*p++ = this_run;
					if ( d->current_heap_transitions == MAX_HEAP_TRANSITIONS )
						break;
					++d->current_heap_transitions;
					this_run = (int32) run_type << 24;
					last_run_type = run_type;
				}
				i = chunk_size >> heap_scale_2n;
				this_run += i;
				chunk_size -= (int32) i << heap_scale_2n;
			}
		}
		else if ( run_type != HEAP_LOCKED_RUN ) {
			if ( block_type == BLOCK_FREE )
				;
			else if ( block_type == BLOCK_NON_RELOCATABLE )
				run_type = HEAP_LOCKED_RUN;
			else {
				tp = (Ptr) zone + ((header_32_t *) bh)->relative_handle;
				if ( tp >= BufPtr || ( (int32) tp & 1 ) )
					goto heap_bad;
				if ( ((header_32_t *) bh)->flags & MASTER_POINTER_LOCK_MASK )
					run_type = HEAP_LOCKED_RUN;
				else if ( ((header_32_t *) bh)->flags & MASTER_POINTER_PURGE_MASK )
					run_type = HEAP_PURGEABLE_RUN;
				else
					run_type = HEAP_UNLOCKED_RUN;
			}
		}

		if ( bh + size <= bh )
			goto heap_bad;
		bh += size;

		if ( bh < zone->bkLim )
			continue;

		if ( bh > zone->bkLim )
			goto heap_bad;

		if ( chunk_size )
			++this_run;
		*p = this_run;
		break;
	}

	d->current_heap_ok = TRUE;
	return;

heap_bad:
	d->current_heap_ok = FALSE;
}
#endif