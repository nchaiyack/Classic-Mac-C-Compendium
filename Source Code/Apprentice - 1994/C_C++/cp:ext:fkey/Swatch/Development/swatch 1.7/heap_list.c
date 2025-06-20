/**

	heap.list.c
	Copyright (c) 1990-1992, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#include <GestaltEqu.h>
#include <Packages.h>

#ifndef __ctypes__
#include "ctypes.h"
#endif
#ifndef __heap__
#include "heap.h"
#endif
#ifndef __heap_list__
#include "heap_list.h"
#endif
#ifndef __linked_list__
#include "linked_list.h"
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

/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

static Boolean system_heap_is_32_bit, process_heaps_are_32_bit;
static Boolean heap_list_inited = false;

static Handle MF_handle;


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

static void init_one( Heap_info_handle_t h, ProcessSerialNumber *psn );


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

Boolean Memory_Manager_is_32_bit;
l_anchor_t heaps;
Heap_info_handle_t multifinder, system;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Heap_list_init( void )
{
	OSErr err;
	int32 response;
	Heap_info_t *hi;

	if ( heap_list_inited ) return;
	heap_list_inited = true;

	if ( Gestalt( gestaltAddressingModeAttr, &response ) ) Bail( BAD_SYSTEM_VERSION_ALRT );
	system_heap_is_32_bit = (response & (1<<gestalt32BitSysZone)) != 0;
	Memory_Manager_is_32_bit =
			process_heaps_are_32_bit = (response & (1<<gestalt32BitAddressing)) != 0;

	MF_handle = TempNewHandle( 0, &err );
	if ( err ) Bail( OUT_OF_MEMORY_ALRT );

	heaps = l_new_list();

	system = l_new( heaps, sizeof(Heap_info_t) );
	init_one( system, nil );

	multifinder = l_new( heaps, sizeof(Heap_info_t) );
	init_one( multifinder, nil );
}


void Heap_list_exit( void )
{
	OSErr err;
	
	MFTempDisposHandle( MF_handle, &err );
	l_old_list( heaps );
}


int16 Heap_list_count( void )
{
	l_elem_t h = nil;
	int count = 0;

	while ( h = l_next( heaps, h ) ) ++count;
	return count;
}


int32 Heap_list_largest_heap_size( void )
{
	int32 largest_heap_size = 0;
	Heap_info_handle_t h = nil;

	while ( (h = l_next( heaps, h )) ) {
		Heap_info_t *hi = l_access( h );
		if ( hi->updating && hi->current_size > largest_heap_size )
			largest_heap_size = hi->current_size;
		l_release( h );
	}

	return largest_heap_size;
}


Boolean Heap_list_update( void )
{
	Heap_info_handle_t h;
	ProcessSerialNumber psn;
	Boolean changed = false;


	psn.highLongOfPSN = 0;
	psn.lowLongOfPSN = kNoProcess;
	while ( !GetNextProcess( &psn ) ) {

		Boolean found = false;

		h = nil;
		while ( !found && (h = l_next( heaps, h )) ) {
			Boolean same;
			Heap_info_t *hi;

			if ( h == multifinder || h == system ) continue;

			hi = l_access( h );
			SameProcess( &psn, &hi->info.processNumber, &same );
			if ( same ) {
				hi->touched = true;
				found = true;
			}

			l_release( h );
		}

		if ( !found ) {
			Heap_info_t *hi;

			h = l_new( heaps, sizeof(Heap_info_t) );
			init_one( h, &psn );

			hi = l_access( h );
			hi->touched = true;
			l_release( h );

			changed = true;
		}

	}


	h = l_next( heaps, nil );
	while ( h ) {

		Boolean touched;
		Heap_info_handle_t h_this;		
		Heap_info_t *hi;

		h_this = h;
		h = l_next( heaps, h );

		if ( h_this == multifinder || h_this == system ) continue;	// never removed

		hi = l_access( h_this );
		touched = hi->touched;
		hi->touched = false;
		l_release( h_this );

		if ( !touched ) {
			l_old( h_this );
			changed = true;
		}

	}


	if ( changed ) {
		if ( !Heap_list_resort() ) Display_update_row_tops();
	}

	return changed;
}


Boolean Heap_list_resort( void )
{
	Heap_info_handle_t h, last_h;
	Heap_info_t *hi;
	int16 last_weight;
	Boolean changed = false;

#if 0
	last_h = h = nil;
	while ( (h = Heap_list_next( h )) ) {

		int16 weight = 0;
		Boolean swapped = false;

		hi = Heap_list_access( h );

		if ( hi->updating ) --weight;

		if ( last_h ) {
			Heap_info_t *last_hi = Heap_list_access( last_h );

			if ( weight < last_weight ) swapped = true;
			else if ( IUCompString( (StringPtr) hi->appname,
					(StringPtr) last_hi->appname ) == 1 ) swapped = true;

			Heap_list_release( last_h );
		}

		Heap_list_release( h );

		if ( swapped ) {
			Linked_list_swap( last_h, h );
			last_h = h = nil;		// restart list
			changed = true;
		}
		else {
			last_weight = weight;
			last_h = h;
		}

	}
#endif

	if ( changed ) Display_update_row_tops();

	return changed;
}


void Heap_list_update_one( Heap_info_handle_t h )
{
	Heap_info_t *hi = l_access( h );

	if ( hi->updating ) {

		if ( h == multifinder ) hi->zone = HandleZone( MF_handle );

		hi->current_size = (int32) hi->zone->bkLim - (int32) &hi->zone->heapData;
		hi->current_free = hi->zone->zcbFree;
		Heap_update_current( h );
	}

	l_release( h );
}



/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/


static void init_one( Heap_info_handle_t h, ProcessSerialNumber *psn )
{
	Heap_info_t *hi = l_access( h );

	hi->updating = true;

	if ( h == multifinder ) {
		pstrcopy( pstr(MultiFinder_STR_x), hi->appname );
		hi->zone = HandleZone( MF_handle );
		hi->heap_is_32_bit = process_heaps_are_32_bit;
		hi->info.processNumber.highLongOfPSN = 0;
		hi->info.processNumber.lowLongOfPSN = kNoProcess;
	}

	else if ( h == system ) {
		pstrcopy( pstr(System_STR_x), hi->appname );
		hi->zone = SysZone;
		hi->heap_is_32_bit = system_heap_is_32_bit;
		hi->info.processNumber.highLongOfPSN = 0;
		hi->info.processNumber.lowLongOfPSN = kNoProcess;
	}

	else {
		hi->info.processNumber = *psn;
		hi->info.processInfoLength = sizeof( ProcessInfoRec );
		hi->info.processName = (StringPtr) hi->appname;
		hi->info.processAppSpec = nil;
		hi->heap_is_32_bit = process_heaps_are_32_bit;
		if ( GetProcessInformation( &hi->info.processNumber, &hi->info ) ) {
			DebugStr( (unsigned char *)"\pinit_one(): GetProcessInformation() failed." );
			return;
		}
		hi->zone = (THz) hi->info.processLocation;
	}

	l_release( h );

	Display_new_row( h );

	Heap_list_update_one( h );
}

