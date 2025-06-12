/**

	swatch.heap.c
	Copyright (c) 1990, Adobe Systems, Inc.

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#include "swatch.h"
#include "swatch.prefs.h"
#include "swatch.heap.h"


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
 ** Private Functions
 **
 **/

void make_24_bit_heap( Display_stat_t *d );

void make_32_bit_heap( Display_stat_t *d );


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

/*******************************************************************************
 ***
 *** prototype
 ***
 *** summary
 ***
 *** History:
 ***
 *** To Do:
 ***
 ***/


void make_current_heap( Display_stat_t *d )
{
	if ( d->heap_is_32_bit )
		make_32_bit_heap( d );
	else
		make_24_bit_heap( d );
}


void make_24_bit_heap( Display_stat_t *d )
{
	register Ptr bh, tp;
	register THz zone;
	register int32 chunk_size, size;
	register uns16 run_type, block_type;
	uns16 last_run_type;
	register int32 this_run;
	int32 *p;
	int32 i;

	zone = d->app.zone;
	bh = (Ptr) &zone->heapData;

	d->current_heap_transitions = 1;
	p = &d->current_heap[0];
	run_type = last_run_type = HEAP_LOCKED_RUN;
	this_run = (int32) run_type << 24;
	chunk_size = 0;

	for (;;) {
		if ( bh >= *(Ptr *) BufPtr || ( (int32) bh & 1 ) )
			goto heap_bad;

		size = *(int32 *) bh & 0x00FFFFFF;
		block_type = *(unsigned char *) bh & BLOCK_TYPE_MASK;
		if ( (chunk_size += size) >= Swatch_prefs.heap_scale ) {
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
				if ( tp >= *(Ptr *) BufPtr || ( (int32) tp & 1 ) )
					goto heap_bad;
				if ( *tp & MASTER_POINTER_LOCK_MASK )
					run_type = HEAP_LOCKED_RUN;
				else if ( *tp & MASTER_POINTER_PURGE_MASK )
					run_type = HEAP_PURGEABLE_RUN;
				else
					run_type = HEAP_UNLOCKED_RUN;
			}
			chunk_size -= Swatch_prefs.heap_scale;
			if ( chunk_size >= Swatch_prefs.heap_scale ) {
				if ( run_type != last_run_type ) {
					*p++ = this_run;
					if ( d->current_heap_transitions == MAX_HEAP_TRANSITIONS )
						break;
					++d->current_heap_transitions;
					this_run = (int32) run_type << 24;
					last_run_type = run_type;
				}
				i = chunk_size >> Swatch_prefs.heap_scale_2n;
				this_run += i;
				chunk_size -= (int32) i << Swatch_prefs.heap_scale_2n;
			}
		}
		else if ( run_type != HEAP_LOCKED_RUN ) {
			if ( !block_type )
				;
			else if ( block_type == BLOCK_NON_RELOCATABLE )
				run_type = HEAP_LOCKED_RUN;
			else {
				tp = (Ptr) zone + *(int32 *) (bh + 4);
				if ( tp >= *(Ptr *) BufPtr || ( (int32) tp & 1 ) )
					goto heap_bad;
				if ( *tp & MASTER_POINTER_LOCK_MASK )
					run_type = HEAP_LOCKED_RUN;
				else if ( *tp & MASTER_POINTER_PURGE_MASK )
					run_type = HEAP_PURGEABLE_RUN;
				else
					run_type = HEAP_UNLOCKED_RUN;
			}
		}

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

void make_32_bit_heap( Display_stat_t *d )
{
	register Ptr tp, bh;
	register THz zone;
	register int32 chunk_size, size;
	register uns16 run_type, block_type;
	uns16 last_run_type;
	register int32 this_run;
	int32 *p;
	int32 i;

	zone = d->app.zone;
	bh = (Ptr) &zone->heapData;

	d->current_heap_transitions = 1;
	p = &d->current_heap[0];
	run_type = last_run_type = HEAP_LOCKED_RUN;
	this_run = (int32) run_type << 24;
	chunk_size = 0;

	for (;;) {
		if ( bh >= *(Ptr *) BufPtr || ( (int32) bh & 1 ) )
			goto heap_bad;

		size = ((header_32_t *) bh)->size;
		block_type = ((header_32_t *) bh)->type;
		if ( (chunk_size += size) >= Swatch_prefs.heap_scale ) {
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
				if ( tp >= *(Ptr *) BufPtr || ( (int32) tp & 1 ) )
					goto heap_bad;
				if ( ((header_32_t *) bh)->flags & MASTER_POINTER_LOCK_MASK )
					run_type = HEAP_LOCKED_RUN;
				else if ( ((header_32_t *) bh)->flags & MASTER_POINTER_PURGE_MASK )
					run_type = HEAP_PURGEABLE_RUN;
				else
					run_type = HEAP_UNLOCKED_RUN;
			}
			chunk_size -= Swatch_prefs.heap_scale;
			if ( chunk_size >= Swatch_prefs.heap_scale ) {
				if ( run_type != last_run_type ) {
					*p++ = this_run;
					if ( d->current_heap_transitions == MAX_HEAP_TRANSITIONS )
						break;
					++d->current_heap_transitions;
					this_run = (int32) run_type << 24;
					last_run_type = run_type;
				}
				i = chunk_size >> Swatch_prefs.heap_scale_2n;
				this_run += i;
				chunk_size -= (int32) i << Swatch_prefs.heap_scale_2n;
			}
		}
		else if ( run_type != HEAP_LOCKED_RUN ) {
			if ( block_type == BLOCK_FREE )
				;
			else if ( block_type == BLOCK_NON_RELOCATABLE )
				run_type = HEAP_LOCKED_RUN;
			else {
				tp = (Ptr) zone + ((header_32_t *) bh)->relative_handle;
				if ( tp >= *(Ptr *) BufPtr || ( (int32) tp & 1 ) )
					goto heap_bad;
				if ( ((header_32_t *) bh)->flags & MASTER_POINTER_LOCK_MASK )
					run_type = HEAP_LOCKED_RUN;
				else if ( ((header_32_t *) bh)->flags & MASTER_POINTER_PURGE_MASK )
					run_type = HEAP_PURGEABLE_RUN;
				else
					run_type = HEAP_UNLOCKED_RUN;
			}
		}

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
