/*
	sillymalloc.c  by James W. Walker, 1991
	
	This is a simple memory allocation scheme for DVIM72-Mac.
	DVIM72-Mac allocates a whole bunch of small chunks of memory,
	does some computation and printing, and frees all of the chunks.
	Therefore no garbage collection is needed, we can essentially
	allocate from a stack.
	
	Public routines:
	
	Ptr		sillymalloc( Size howmuch );
	
	This routine attempts to allocate "howmuch" memory.  Memory is first
	allocated as nonrelocatable memory in the application heap, then
	when that runs out we try to use MultiFinder temporary memory.
	The latter is not strictly kosher; MultiFinder memory is supposed to
	be so temporary that you don't process events while it's around.
	If we can't get the memory, the result is NIL.
	
	void		sillyfree( void );
	
	This routine frees ALL of the memory allocated by sillymalloc.
	
	void		silly_get_MF_mem_status( void );
	
	This sets the variable MF_mem_status.

 */

#define	TRACE 0

#if TRACE
#include <stdio.h>
#include "TextDisplay.h"
#endif

#include "sillymalloc.h"

extern Boolean TrapAvailable( short trapnum );
extern void	Update_memory_indicators( void );

#ifndef NIL
#define NIL 0L
#endif

#define MIN_POOL  		32000L
#define RESERVE_MEM		60000L

#ifndef MAX
#define MAX(a,b)  ((a)<(b) ? b : a)
#endif


typedef struct sillypool {
	struct sillypool 	*next;		/* Ptr to next pool */
	Ptr 				freeplace;	/* Ptr to first free byte */
	Handle				MF_handle;	/* handle if this is MF mem, else NIL */
	Size				size;		/* How big is storage? */
	char 				storage[];	/* start of actual free space */
} sillypool;

static sillypool *top_pool = NIL;	/* Start of our list of pools. */
static long reserve_heap;	/* leave some heap space for the driver, etc. */
static long reserve_temp_mem;	// leave some multifinder memory for safety
#if TRACE
static long total_allocated;
#endif

sm_temp_mem_status	MF_mem_status = sm_unknown;

/* prototypes of private routines */
static sillypool	*sillynewpool( Size needed );
static OSErr		growsillypool( Size newsize );
void		silly_get_MF_mem_status( void );

#define		DEBUG		0

#if DEBUG
#define		CKPT(x)		DebugStr( "\p" x ";hc" );
#define		ASSERT(x,y)		if (!(x)) {DebugStr("\p" y);}
#else
#define		CKPT(x)
#define		ASSERT(x,y)
#endif


/* ----------------------- sillyfree ------------------------------- */
//	Free all memory pools.
void sillyfree( void )
{
	register  sillypool *dead;
	OSErr				err;

#if TRACE
	TD_printf( "\ntotal allocated: %ld.", total_allocated );
#endif
	while (top_pool != NIL)
	{
		dead = top_pool;
#if TRACE
		TD_printf( "\nfree pool %ld, %ld in use.", dead->size,
			dead->freeplace - StripAddress((Ptr)&dead->storage) );
#endif
		top_pool = top_pool->next;
		if (dead->MF_handle == NIL)
			DisposPtr( (Ptr) dead );
		else
			MFTempDisposHandle( dead->MF_handle, &err );
	}
	Update_memory_indicators();
}


/* ----------------------- sillymalloc ------------------------------- */
Ptr sillymalloc( register Size howmuch )
{
	Ptr  newmem;
	Size  shortage;
	sillypool *newpool;
	OSErr		err;

	CKPT("malloc begin");
	howmuch = (howmuch + 1L) & 0xFFFFFFFE; /* round to next even number */
#if TRACE
	TD_printf( " (malloc %ld", howmuch );
#endif
	if (top_pool == NIL)	/* Get the first pool. */
	{
#if TRACE
		total_allocated = 0L;
#endif
		if (MF_mem_status == sm_unknown)
			silly_get_MF_mem_status();
		top_pool = sillynewpool( howmuch );
		if (top_pool == NIL)
		{
#if TRACE
			TD_printf( " NIL pool)\n" );
#endif
			CKPT( "malloc NIL pool end" );
			return NIL;
		}
		top_pool->next = NIL;
    }
	/* Now we have a non-null top pool. */
	shortage = top_pool->freeplace + howmuch -
		( (Ptr)(top_pool->storage) + top_pool->size );
	if (shortage > 0)
	{	/* Not enuf room, try to grow pool */
		err = growsillypool( MAX(shortage, MIN_POOL) );
		if (err != noErr)	/* Can't grow, try for another. */
		{
			newpool = sillynewpool( howmuch );
			if (newpool == NIL)
			{
#if TRACE
				TD_printf( " NIL pool)\n" );
#endif
				CKPT("malloc NIL end");
				return NIL;
			}
			newpool->next = top_pool;
			top_pool = newpool;
		}
	}
	newmem = top_pool->freeplace;
	top_pool->freeplace += howmuch;
	ASSERT( top_pool->freeplace <= (Ptr)top_pool->storage + top_pool->size,
		"malloc fail sanity check" );
#if TRACE
	total_allocated += howmuch;
	TD_printf( ")\n" );
#endif
	CKPT("malloc good end");
	return newmem;
}

/* 
	Private routine to allocate a new memory pool.
*/
/* ----------------------- sillynewpool ------------------------------- */
static sillypool	*sillynewpool( Size needed )
{
	sillypool	*pool;
	Handle		MF_temp;
	OSErr		err;
	Size		this_much;
	
	CKPT("newpool begin");
	this_much = MAX( needed + sizeof(sillypool), MIN_POOL );
#if TRACE
	TD_printf( " (newpool %ld, %ld", needed, this_much );
#endif
	if (this_much + reserve_heap < FreeMem())
	{
		pool = (sillypool *) NewPtr( this_much );
#if TRACE
		if (pool == NIL)
			TD_printf( " NewPtr failed" );
		else if (MemError() != noErr)
			TD_printf( " NewPtr error" );
		else
			TD_printf( " from app heap" );
#endif
		ASSERT( (pool == NIL) || (MemError() != noErr), "NewPtr error" );
	}
	else
	{
		pool = NIL;
	}

	if (pool == NIL)
	{
		if ( (MF_mem_status == sm_no_temp_mem) ||
			(this_much + reserve_temp_mem > MFFreeMem()) )
		{
#if TRACE
			TD_printf( " no temp mem)\n" );
#endif
			CKPT( "newpool NIL end" );
			return NIL;
		}
		MF_temp = MFTempNewHandle( this_much, &err );
		ASSERT( (MF_temp == NIL) || (err != noErr),
			"MFTempNewHandle error" );
		if (MF_temp == NIL)
		{
#if TRACE
			TD_printf( " temp new failed)\n" );
#endif
			CKPT("newpool NIL end");
			return NIL;
		}
#if TRACE
		else if (err != noErr)
			TD_printf( " temp new error %d", err );
		else
			TD_printf( " from temp mem" );
#endif
		MFTempHLock( MF_temp, &err );
#if TRACE
		if (err != noErr)
			TD_printf( " temp lock error %d", err );
#endif
		ASSERT( (MF_temp == NIL) || (err != noErr),
			"MFTempHLock error" );
		pool = (sillypool *) *MF_temp;
		pool->MF_handle = MF_temp;
	}
	else
		pool->MF_handle = NIL;
	pool = (sillypool *) StripAddress( (Ptr) pool );
	pool->freeplace = (Ptr) (pool->storage);
	pool->size = this_much - sizeof(sillypool);
	Update_memory_indicators();
#if TRACE
	TD_printf( ")\n" );
#endif
	CKPT("newpool good end");
	return pool;
}


/* ----------------------- growsillypool ------------------------------- */
static OSErr		growsillypool( Size more )
{
	OSErr	err;
	Size	new_size;
	THz		save_zone;
	
	CKPT("growpool begin");
	new_size = sizeof(sillypool) + top_pool->size + more;
#if TRACE
	TD_printf( " (grow %ld to %ld", more, new_size );
#endif
	if (top_pool->MF_handle != NIL)
	{
		if (MF_mem_status == sm_temp_mem_is_real)
		{
			save_zone = GetZone();
			SetZone( HandleZone( top_pool->MF_handle ) );
			SetHandleSize( top_pool->MF_handle, new_size );
			err = MemError();
			SetZone( save_zone );
			if (err == noErr)
			{
#if TRACE
				TD_printf( " SetHandleSize OK" );
#endif
				top_pool->size = top_pool->size + more;
			}
#if TRACE
			else
				TD_printf( " SetHandleSize error %d", err );
#endif
		}
		else
			err = memFullErr;	/* Can't resize MF temporary memory. */
	}
	else
	{
		if (more + reserve_heap < FreeMem())
		{
			SetPtrSize( (Ptr)top_pool, new_size );
			err = MemError();
			if (err == noErr)
			{
				top_pool->size = top_pool->size + more;
#if TRACE
				TD_printf( " SetPtrSize OK" );
#endif
			}
#if TRACE
			else
				TD_printf( " SetPtrSize error %d", err );
#endif
		}
		else
			err = memFullErr; /* not enuf room to resize */
	}
	Update_memory_indicators();
#if TRACE
	TD_printf( ")\n" );
#endif
	CKPT("growpool end");
	return err;
}

#ifndef OSDISPATCH
#define OSDISPATCH		0xA88F
#endif

#ifndef GESTALT
#define GESTALT			0xA1AD
#endif
/* ------------------------ silly_get_MF_mem_status ------------------- */
void		silly_get_MF_mem_status( void )
{
	long	response;
	OSErr	err;
	long	**reserve_heap_h, **reserve_temp_mem_h;
	
	MF_mem_status = TrapAvailable( OSDISPATCH ) ?
		sm_temp_mem : sm_no_temp_mem;
#if TRACE
	if (MF_mem_status == sm_temp_mem)
		TD_printf( " (temp mem is available" );
#endif
	
	if ( TrapAvailable(GESTALT) )
	{
		err = Gestalt( gestaltOSAttr, &response );
		if ( (err == noErr) && (response & (1L << gestaltRealTempMemory)) )
			MF_mem_status = sm_temp_mem_is_real;
#if TRACE
		if (MF_mem_status == sm_temp_mem_is_real)
			TD_printf( " and real" );
#endif
	}
	
	reserve_heap_h = (long **) GetNamedResource( 'LONG', "\pheap reserve" );
	if (reserve_heap_h != NIL)
		reserve_heap = **reserve_heap_h;
#if TRACE
	else
		TD_printf(" (heap reserve resource missing)" );
	TD_printf( ")\n" );
#endif
	reserve_temp_mem_h = (long **)GetNamedResource('LONG', "\pMF mem reserve" );
	if (reserve_temp_mem_h != NIL)
		reserve_temp_mem = **reserve_temp_mem_h;
}
