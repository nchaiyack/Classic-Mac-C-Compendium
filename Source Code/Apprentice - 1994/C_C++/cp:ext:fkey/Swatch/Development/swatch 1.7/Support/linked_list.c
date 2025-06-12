/**

	linked_list.c
	Copyright (c) 1992, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#define MEMORY_DEBUG	1


#ifndef __linked_list__
#include "linked_list.h"
#endif
#ifndef __pstring__
#include "pstring.h"		// for blockzero()
#endif


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

struct _t_header {
	int16 lock_count;
	struct _t_header **prev, **next;
#if MEMORY_DEBUG
	void *allocated_by;
	void *last_accessed_by;
#endif
};


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

l_anchor_t l_new_list( void )
{
	OSErr err;
	l_anchor_t anchor;

	anchor = (l_anchor_t) TempNewHandle( sizeof(struct _t_header), &err );
	if ( anchor ) (**anchor).next = (**anchor).prev = anchor;

	return anchor;
}


void l_old_list( l_anchor_t anchor )
{
	OSErr err;

	while ( (**anchor).next != anchor ) l_old( (**anchor).next );
	TempDisposeHandle( (Handle) anchor, &err );	
}


l_elem_t l_new( l_anchor_t anchor, int32 size )
{
	OSErr err;
	l_elem_t h;
	struct _t_header *hi;

	size += sizeof(struct _t_header);
	h = (l_elem_t) TempNewHandle( size, &err );
	if ( h ) {

		HLock( (Handle) h );
		hi = *h;
		blockzero( (unsigned char *) hi, size );  // zeros lock count, too

#if MEMORY_DEBUG
		{
		void *temp;
		asm {
			move.l	4(A6), temp
		}
		hi->allocated_by = temp;
		}
#endif

#if 0
		hi->prev = anchor;
		hi->next = (**anchor).next;
		(**(**anchor).next).prev = h;
		(**anchor).next = h;
#else
		hi->next = anchor;
		hi->prev = (**anchor).prev;
		(**(**anchor).prev).next = h;
		(**anchor).prev = h;
#endif

		HUnlock( (Handle) h );

	}

	return h;
}


void l_old( l_elem_t h )
{
	(**(**h).prev).next = (**h).next;
	(**(**h).next).prev = (**h).prev;

	DisposHandle( (Handle) h );
}


void *l_access( l_anchor_t h )
{
	if ( !(**h).lock_count++ ) HLock( (Handle) h );

#if MEMORY_DEBUG
	{
	void *temp;
	asm {
			move.l	4(A6), temp
	}
	(**h).last_accessed_by = temp;
	}
#endif

	return StripAddress( (Ptr)*h + sizeof(struct _t_header) );
}


void l_release( l_anchor_t h )
{
	if ( !(**h).lock_count ) {
		DebugStr( (unsigned char *)"\pHeap_l_unlock(): too many unlocks" );
		return;
	}

	if ( !--(**h).lock_count ) HUnlock( (Handle) h );
}


l_elem_t l_next( l_anchor_t anchor, l_elem_t this )
{
	l_elem_t next;

	if ( !this ) next = (**anchor).next;
	else next = (**this).next;

	if ( next == anchor ) return nil;
	else return next;
}


l_elem_t l_prev( l_anchor_t anchor, l_elem_t this )
{
	l_elem_t prev;

	if ( !this ) prev = (**anchor).prev;
	else prev = (**this).prev;

	if ( prev == anchor ) return nil;
	else return prev;
}
