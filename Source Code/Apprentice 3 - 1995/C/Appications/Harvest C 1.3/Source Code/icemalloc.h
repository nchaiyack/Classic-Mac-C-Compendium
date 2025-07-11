/*
**
** Notice.
**
** This source code was written by Tim Endres. time@ice.com
** Copyright 1988-1991 � By Tim Endres. All rights reserved.
**
** You may use this source code for any purpose you can dream
** of as long as this notice is never modified nor removed.
**
*/

#pragma once

/*
** Right now we are limiting the maximum single allocation unit to 16Meg.
** This way we can stuff the index to the next ptr hdr into the
** low 24 bits of a long, then slam 8 bits of flag information
** into the upper 8 bits of the same long. Not necessarily beautiful
** but compact and functional.
*/

/*
**	_PM_DEBUG_LEVEL
**
**  1 - DPRINTF ERROR conditions.
**  2 - DPRINTF *AND* DACTION ERROR conditions.
**
**  3 - DPRINTF WARNING conditions.
**  5 - DPRINTF DEBUGING conditions.
** 10 - DPRINTF NOTES conditions.
**
*/

#ifdef DEBUG

#	define _PM_STATS
#	define _PM_DYNAMIC_MERGING
#	define _PM_DYNAMIC_FREE

#	define	_PM_DEBUG_LEVEL		1

#	define DPRINTF(level, parms)	{ if ((level) <= pool_malloc_debug_level) { printf parms ; } }
#	define DACTION(level, action)	{ if ((level) <= pool_malloc_debug_level) { action } }

int		pool_malloc_debug_level = _PM_DEBUG_LEVEL;

#else

#	define _PM_DYNAMIC_MERGING
#	define _PM_DYNAMIC_FREE

#	define	_PM_DEBUG_LEVEL		0

#	define DPRINTF(level, parms)
#	define DACTION(level, action)

#endif DEVELOPMENT


/*
** MEMORY PTR HEADER FLAG BITS:
**
** 01 _PM_PTR_FREE		Is this piece of memory free?
** 02
** 04
** 08
**
** 10
** 20
** 40
** 80 _PM_PTR_PARITY	This is a parity bit for debugging.
**
*/

#define _PM_PTR_USED		0x01
#define _PM_PTR_PARITY		0x80

#define _PM_MIN_ALLOC_SIZE	8

#define	ALIGNMENT				4		/* The 68020 likes things long aligned. */
#define INT_ALIGN(i, r)			( ((i) + ((r) - 1)) & ~((r) - 1) )


#define SUGGESTED_BLK_SIZE		8192


#define GET_PTR_FLAGS(hdr)	\
	( (u_long) ( (((hdr)->size) >> 24) & 0x000000FF ) )
#define SET_PTR_USED(hdr)	\
	( (hdr)->size |= (((_PM_PTR_USED) << 24) & 0xFF000000) )
#define SET_PTR_FREE(hdr)	\
	( (hdr)->size &= ~(((_PM_PTR_USED) << 24) & 0xFF000000) )
#define IS_PTR_USED(hdr)	\
	( (GET_PTR_FLAGS(hdr) & _PM_PTR_USED) != 0 )
#define IS_PTR_FREE(hdr)	\
	( (GET_PTR_FLAGS(hdr) & _PM_PTR_USED) == 0 )

#define GET_PTR_SIZE(hdr)	\
	( (u_long) ( ((hdr)->size) & 0x00FFFFFF ) )
#define SET_PTR_SIZE(hdr, blksize)	\
	( (hdr)->size = ( ((hdr)->size & 0XFF000000) | ((blksize) & 0x00FFFFFF) ) )

typedef unsigned long	u_long;
typedef unsigned char	u_char;
typedef unsigned int	u_int;
typedef unsigned short	u_short;

typedef struct {
	u_long		size;
	} _mem_ptr_hdr, *_mem_ptr_hdr_ptr;


typedef struct _MEM_BLK {
	u_long				size;			/* The size of this block's memory. */
	char				*memory;		/* The block's allocated memory. */
	u_long				max_free;		/* The maximum free size in the block */
	struct _MEM_BLK		*next;			/* The next block in the pool block list. */
	struct _MEM_POOL	*pool;			/* The block's pool. */
	} _mem_blk, *_mem_blk_ptr;


typedef struct _MEM_POOL {
	int					id;				/* The pool's ID. */
	u_long				pref_blk_size;	/* The preferred size of new blks. */
	_mem_blk_ptr		blk_list;		/* The list of blocks in the pool. */
	struct _MEM_POOL	*next;			/* The next pool in the forest. */
#ifdef _PM_STATS
	u_long				total_memory;	/* The total allocated memory by this pool */
	u_long				total_storage;	/* The total malloc-able storage in this pool */
	u_long				total_malloc;	/* The total malloc-ed storage not freed. */
	u_long				max_blk_size;	/* The maximum block size allocated. */
	float				ave_req_size;	/* The ave allocated request size */
	u_long				ave_req_total;	/* The total requests in the average. */
	float				ave_blk_size;	/* The ave sallocated blk size */
	u_long				ave_blk_total;	/* The total blks in the average. */
#endif
	} _mem_pool, *_mem_pool_ptr;



static _mem_pool	_mem_system_pool = {
	0,						/* id */
	SUGGESTED_BLK_SIZE,		/* pref_blk_size */
	0,						/* blk_list */
	0,						/* next */
#ifdef _PM_STATS
	0,						/* total_memory */
	0,						/* total_storage */
	0,						/* total_malloc */
	0,						/* max_blk_size */
	0.0,					/* ave_req_size */
	0,						/* ave_req_total */
	0.0,					/* ave_blk_size */
	0,						/* ave_blk_total */
#endif
	};

/*
** The memory pool forest. To the user, this is simply a disjoint
** group of memory pools, in which his memory pools lie. We keep
** it as a simple linked list. Forward linked, nothing fancy.
**
** The default pool is simply the front pool in the pool forest list.
*/
extern _mem_pool_ptr				_mem_pool_forest;

#define _default_mem_pool	_mem_pool_forest


char				*icemalloc();
int					icefree();
char				*_pool_malloc();
_mem_blk_ptr		_pool_new_blk();
_mem_blk_ptr		_pool_find_free_blk();
_mem_blk_ptr		_pool_find_ptr_blk();
_mem_ptr_hdr_ptr	_blk_find_free_hdr();
_mem_pool_ptr		find_pool();
_mem_pool_ptr		new_malloc_pool();

