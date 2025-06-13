/*
**
** Notice.
**
** This source code was written by Tim Endres. time@ice.com
** Copyright 1988-1991 © By Tim Endres. All rights reserved.
** 8840 Main Street, Whitmore Lake, MI  48189
**
** You may use this source code for any purpose you can dream
** of as long as this notice is never modified nor removed.
**
** Please email me any improvements that you might make. Any
** reasonable form of diff (compare) on the files changes will
** do nicely. You can mail "time@ice.com". Thank you.
**
** BTW: In case it is not obvious, do not #define DOCUMENTATION ;)
**
*/


#include "icemalloc.h"
_mem_pool_ptr				_mem_pool_forest = & _mem_system_pool;

#define mNEWPTR(size)			( NewPtrClear ( (size) ) )
#define mDISPOSPTR(ptr)			( DisposPtr ( (ptr) ) )

#define DEBUG

#ifdef DOCUMENTATION

	malloc() will allocate "size" bytes from the default malloc pool.
	
#endif

char	*
icemalloc ( size )
u_long	size;
{
_mem_pool_ptr	pool;

	pool = _default_mem_pool;
	if (pool == (_mem_pool_ptr)0)
		return (char *)0;
	
	return _pool_malloc(pool, size);
	}


#ifdef DOCUMENTATION

	free() will free the memory occupied by the "ptr" allocated by malloc().
	
#endif

icefree ( ptr )
char	*ptr;
{
	return _pool_free(ptr);
	}


#ifdef DOCUMENTATION

	set_default_pool() - given a pool id, this routine will make it the default
	pool from which malloc() allocates memory.
	
#endif

set_default_pool ( id )
int		id;
{
_mem_pool_ptr	pool, last;

	if (_default_mem_pool->id == id)
		return 0;
	
	last = _default_mem_pool;
	pool = _default_mem_pool->next;
	for ( ; pool != (_mem_pool_ptr)0 ; pool = pool->next) {
		if (pool->id == id) {
			last->next = pool->next;
			pool->next = _default_mem_pool;
			_default_mem_pool = pool;
			return 0;
			}
		}
	
	return -1;
	}


#ifdef DOCUMENTATION

	new_malloc_pool() creates a new pool from which memory can be malloc()-ed.
	
#endif

_mem_pool_ptr
new_malloc_pool ( id, pref_blk_size )
int		id;
u_long	pref_blk_size;
{
_mem_pool_ptr	new_pool;

	new_pool = find_pool(id);
	if (new_pool != NULL)			/* ? Is this the best choice? Its not ID-able */
		return new_pool;

	new_pool = (_mem_pool_ptr) mNEWPTR( sizeof(_mem_pool) );
	if (new_pool == (_mem_pool_ptr)0)
		return new_pool;
	
	new_pool->id = id;							/* The pool's ID. */
	new_pool->pref_blk_size = pref_blk_size;	/* The preferred size of new blks. */
	new_pool->blk_list = NULL;					/* The list of blocks in the pool. */
	
	/* The next two lines insert right after the default, so we don't change it. */
	new_pool->next = _mem_pool_forest->next;
	_mem_pool_forest->next = new_pool;

#ifdef _PM_STATS
	new_pool->total_memory = 0;			/* The total allocated memory by this pool */
	new_pool->total_storage = 0;		/* The total malloc-able storage in this pool */
	new_pool->total_malloc = 0;			/* The total malloc-ed storage not freed. */
	new_pool->max_blk_size = 0;			/* The maximum block size allocated. */
	new_pool->ave_req_size = 0.0;		/* The ave allocated request size */
	new_pool->ave_req_total = 0;		/* The total requests in the average. */
	new_pool->ave_blk_size = 0.0;		/* The ave sallocated blk size */
	new_pool->ave_blk_total = 0;		/* The total blks in the average. */
#endif

	return new_pool;
	}


#ifdef DOCUMENTATION

	find_pool() will find the pool with the given "id" and return its pointer.
	
#endif

_mem_pool_ptr
find_pool ( id )
int		id;
{
_mem_pool_ptr	pool;

	for (pool = _mem_pool_forest ; pool != (_mem_pool_ptr)0 ; pool = pool->next) {
		if (pool->id == id)
			break;
		}
		
	return pool;
	}


#ifdef DOCUMENTATION

	free_pool_memory() this will free and *release* all memory occupied by the
	pool but not free the pool, letting you allocate some more.
	
#endif

free_pool_memory ( id )
int		id;
{
_mem_pool_ptr	pool;
_mem_blk_ptr	blk, nextblk;

	pool = find_pool(id);
	if (pool == NULL)
		return -1;
	
	for ( blk = pool->blk_list ; blk != (_mem_blk_ptr)0 ; ) {
		nextblk = blk->next;
		DPRINTF(3, ("pool_find_free_blk() Freeing Block x%lx from pool #%d!\n",
						blk, blk->pool->id));
#ifdef _PM_STATS
		pool->total_memory -= sizeof(_mem_blk) + blk->size;
		pool->total_storage -= blk->size;
#endif
		mDISPOSPTR((Ptr)blk->memory);
		mDISPOSPTR((Ptr)blk);

		blk = nextblk;
		}

	pool->blk_list = NULL;
	
	return 0;
	}


#ifdef DOCUMENTATION

	free_pool() will free the pool's memory *and* free the pool (removing
	it from the pool list) releasing all memory back to the heap.
	
#endif

free_pool ( id )
int		id;
{
_mem_pool_ptr	pool;

	if (free_pool_memory(id) == -1)
		return -1;
	
	pool = find_pool(id);
	if (pool == NULL)
		return -1;
	
	mDISPOSPTR((Ptr)pool);
	
	return 0;
	}


#ifdef DOCUMENTATION

	_pool_malloc() does the low level malloc() work in a specified pool.
	
#endif

char	*
_pool_malloc(pool, size)
_mem_pool_ptr	pool;
u_long			size;
{
_mem_blk_ptr		blk;
_mem_ptr_hdr_ptr	hdr = (_mem_ptr_hdr_ptr)0, freehdr;
u_long				freesize, size_req;
char				*ptr = (char *)0;

	DPRINTF(5, ("_pool_malloc() request of %ld bytes in pool #%d [x%lx]\n",
				size, pool->id, pool));

	size_req = (size < _PM_MIN_ALLOC_SIZE) ? _PM_MIN_ALLOC_SIZE : size;
	size_req = INT_ALIGN(size_req, ALIGNMENT);

#ifdef _PM_STATS
	pool->ave_req_size = ( ( (pool->ave_req_size * (float)pool->ave_req_total) + (float)size_req )
							/ (float)(pool->ave_req_total + 1) );
	pool->ave_req_total++;
#endif

	blk = pool->blk_list;
	if (blk == NULL) {
		/* No blocks in pool, allocate one... */
		blk = _pool_new_blk(pool, size_req);
		}
	else {
		blk = _pool_find_free_blk(pool, size_req, &hdr);

		if (blk == (_mem_blk_ptr)0 || hdr == (_mem_ptr_hdr_ptr)0) {
			/* No blocks that can support this size... */
			blk = _pool_new_blk(pool, size_req);
			}
		else
			DPRINTF(5, ("_pool_malloc() found free: blk x%lx hdr x%lx\n",
						blk, hdr));
		}
	
	if (blk != (_mem_blk_ptr)0) {
		/* Determine the pointer's location, establish, return. */
		if (hdr == (_mem_ptr_hdr_ptr)0) {
			hdr = (_mem_ptr_hdr_ptr) blk->memory;
			DPRINTF(5, ("_pool_malloc() header of new blk blk->memory x%lx\n", blk->memory));
			}
		
		DPRINTF(5, ("_pool_malloc() free hdr x%lx size %ld\n", hdr, GET_PTR_SIZE(hdr)));
		if (hdr != (_mem_ptr_hdr_ptr)0) {
			ptr = (char *)hdr + sizeof(_mem_ptr_hdr);
			
			if (size_req < GET_PTR_SIZE(hdr)) {
				/* Split this free block... */
				DPRINTF(5, ("_pool_malloc() split hdr x%lx into %ld used and %ld free\n",
							hdr, size_req, GET_PTR_SIZE(hdr) - (size_req + sizeof(_mem_ptr_hdr))));

				freehdr = (_mem_ptr_hdr_ptr)
							( (char *)hdr + sizeof(_mem_ptr_hdr) + size_req );
				freesize = GET_PTR_SIZE(hdr) - (sizeof(_mem_ptr_hdr) + size_req);
				bzero(freehdr, sizeof(_mem_ptr_hdr));
				SET_PTR_FREE(freehdr);
				SET_PTR_SIZE(freehdr, freesize);
				blk->max_free -= sizeof(_mem_ptr_hdr);
				}

#ifdef _PM_STATS	
			pool->total_malloc += size_req;
#endif

			blk->max_free -= size_req;
			SET_PTR_USED(hdr);
			SET_PTR_SIZE(hdr, size_req);
			bzero(ptr, size_req);		/* Programmer's expect malloc() to zero. */
			}
		else {
			/* ERROR: This should not happen!!! */
			DPRINTF(1, ("ERROR: pool_malloc() could not get block's free hdr ptr\n"));
			DACTION(2, { list_pool_forest(NULL); });
			}
		}
	else {
		/* ERROR, no block, no memory. */
		DPRINTF(1, ("ERROR: pool_malloc() could not get a block\n"));
		DACTION(2, { list_pool_forest(NULL); });
		}
	
	DPRINTF(5, ("_pool_malloc() returning ptr x%lx\n", ptr));
	return ptr;
	}

#ifdef DOCUMENTATION

	_pool_free() does the low level work of a free().
	
#endif

_pool_free ( ptr )
char	*ptr;
{
_mem_pool_ptr		pool;
_mem_blk_ptr		blk;
_mem_ptr_hdr_ptr	hdr, adjhdr, limit;
int					result = 0;
u_long				ptr_size, new_size;

	if (ptr == (char *)0) {
		/* WHOAH! NULL Pointers... */
		DPRINTF(1, ("_pool_free() ptr NULL!"));
		return -1;
		}
	
	if ( ( (u_long)ptr & 1 ) != 0 ) {
		/* WHOAH! ODD Pointers... */
		DPRINTF(1, ("_pool_free() ptr ODD!"));
		return -1;
		}
	
	DPRINTF(5, ("_pool_free() free ptr x%lx\n", ptr));

	blk = _pool_find_ptr_blk(ptr);
	
	if (blk == (_mem_blk_ptr)0) {
		/* We could not find this thing's blk! BUG!!! */
		DPRINTF(1, ("ERROR: free(x%lx) could not find ptr's blk\n", ptr));
		DACTION(2, { list_pool_forest(NULL); });
		result = -1;
		}
	else {
		/*
		** Now, if it is adjacent to free memory, combine.
		** NOTE: We only do this because it is SO DAMN CHEAP!!!!!
		*/

		/* Delay these assigns til we know ptr is good. (now) */
		hdr = (_mem_ptr_hdr_ptr) ( (u_long)ptr - sizeof(_mem_ptr_hdr) );
		ptr_size = GET_PTR_SIZE(hdr);

		DPRINTF(5, ("_pool_free() free hdr x%lx size %ld flags x%02x in blk x%lx\n",
					hdr, ptr_size, GET_PTR_FLAGS(hdr), blk));
	
		pool = blk->pool;
		SET_PTR_FREE(hdr);
		blk->max_free += ptr_size;
		
		adjhdr = (_mem_ptr_hdr_ptr)
					( (char *)hdr + GET_PTR_SIZE(hdr) +
					  sizeof(_mem_ptr_hdr) );
		limit = (_mem_ptr_hdr_ptr) ((char *)blk->memory + blk->size);
		
		if (adjhdr < limit && IS_PTR_FREE(adjhdr)) {
			DPRINTF(5, ("_pool_free() merging hdr x%lx with freehdr x%lx\n", hdr, adjhdr));
			
			new_size =	GET_PTR_SIZE(hdr) +
						GET_PTR_SIZE(adjhdr) +
						sizeof(_mem_ptr_hdr);

			DPRINTF(5, ("_pool_free() merged hdr new_size %ld\n", new_size));
			SET_PTR_SIZE(hdr, new_size);
			blk->max_free += sizeof(_mem_ptr_hdr);
			}	/* is adjacent free ? */
		
#ifdef _PM_STATS	
		pool->total_malloc -= ptr_size;
#endif

#ifdef _PM_DYNAMIC_FREE
		if (block_is_freed(blk)) {
			/* This blk is free-ed ... */
			_mem_blk_ptr	tmpblk;

			if (blk == blk->pool->blk_list) {
				blk->pool->blk_list = blk->next;
				}
			else {
				tmpblk = blk->pool->blk_list;
				while (tmpblk != (_mem_blk_ptr)0) {
					if (tmpblk->next == blk) {
						tmpblk->next = blk->next;
						break;
						}
					else
						tmpblk = tmpblk->next;
					}
				if (tmpblk == (_mem_blk_ptr)0)
					DPRINTF(1, ("ERROR: could not free blk x%lx from list!\n", blk));
				}
			
			DPRINTF(3, ("_pool_free() Freeing Block x%lx from pool #%d!\n",
						blk, blk->pool->id));
#ifdef _PM_STATS
			pool->total_memory -= sizeof(_mem_blk) + blk->size;
			pool->total_storage -= blk->size;
#endif
			mDISPOSPTR((Ptr)blk->memory);
			mDISPOSPTR((Ptr)blk);
			}	/* if (block_is_freed(blk)) */
#endif _PM_DYNAMIC_FREE

		result = 0;
		}	/* else we found the block containing the ptr. */
	
	return result;
	}

#ifdef DOCUMENTATION

	block_is_freed() determines if all memory in a given block is free.
	
#endif

block_is_freed(blk)
_mem_blk_ptr	blk;
{
int			freed = 1;
_mem_ptr_hdr_ptr	loophdr, limit;

/*
** This loop is not as expensive as you might think, since most of the
** time we've "merged" into few "ptrs", and in allocated cases we will
** break out of the loop almost immediately.
*/

	if (blk != NULL) {
		loophdr = (_mem_ptr_hdr_ptr) blk->memory;
		limit = (_mem_ptr_hdr_ptr) ((char *)blk->memory + blk->size);
		while (loophdr < limit) {
			if (! IS_PTR_FREE(loophdr)) {
				freed = 0;
				break;
				}
			
			loophdr = (_mem_ptr_hdr_ptr)
						((char *)loophdr + GET_PTR_SIZE(loophdr) + sizeof(_mem_ptr_hdr));
			}
		}
	else
		freed = 0;	/* ? Or is it really free? */
	
	return freed;
	}


#ifdef DOCUMENTATION

	_pool_new_blk() allocate a new memory block to a pool. This is called
	by the low level malloc() code when new memory is needed.
	
#endif

_mem_blk_ptr
_pool_new_blk(pool, size_req)
_mem_pool_ptr	pool;
u_long			size_req;	/* size required */
{
u_long				blk_size;
_mem_blk_ptr		new_blk;
_mem_ptr_hdr_ptr	hdr;

	DPRINTF(5, ("_pool_new_blk() req_size %ld pool #%d [x%lx]\n", size_req, pool->id, pool));

	if ((size_req + sizeof(_mem_ptr_hdr)) > pool->pref_blk_size) {
		blk_size = ((size_req / pool->pref_blk_size) + 1) * pool->pref_blk_size;
#ifdef _PM_STATS
		if (blk_size > pool->max_blk_size)
			pool->max_blk_size = blk_size;
#endif
		}
	else
		blk_size = pool->pref_blk_size;
	
	blk_size = INT_ALIGN(blk_size, ALIGNMENT);
	
#ifdef _PM_STATS
	pool->ave_blk_size = ( ( (pool->ave_blk_size * (float)pool->ave_blk_total) + (float)blk_size )
							/ (float)(pool->ave_blk_total + 1) );
	pool->ave_blk_total++;
#endif

	new_blk = (_mem_blk_ptr) mNEWPTR(sizeof(_mem_blk));
	if (new_blk == (_mem_blk_ptr)0)
		return new_blk;
	
	/*
	** NOTE: We assume here that mNEWPTR() gives us proper alignment.
	*/
	new_blk->memory = (char *) mNEWPTR(blk_size);
#ifdef NEVER_DEFINED
	if (new_blk->memory == (char *)0) {
		/* Attempt to handle the request only. */
		blk_size = size_req + sizeof(_mem_ptr_hdr);
		blk_size = INT_ALIGN(blk_size, ALIGNMENT);
		new_blk->memory = (char *) mNEWPTR(blk_size);
		}
#endif
	
	if (new_blk->memory == (char *)0) {
		mDISPOSPTR((Ptr)new_blk);
		DPRINTF(10, ("_pool_new_blk(pool:x%lx, size:%ld) Out of memory.\n", pool, size_req));
		return (_mem_blk_ptr)0;
		}
	
#ifdef _PM_STATS
	pool->total_memory += sizeof(_mem_blk) + blk_size;
	pool->total_storage += blk_size;
#endif

	new_blk->pool = pool;
	new_blk->size = blk_size;
	new_blk->max_free = blk_size - sizeof(_mem_ptr_hdr);
	
	/* Add to the block list. */
	new_blk->next = pool->blk_list;
	pool->blk_list = new_blk;
	
	hdr = (_mem_ptr_hdr_ptr) new_blk->memory;
	bzero(hdr, sizeof(_mem_ptr_hdr));
	
	SET_PTR_FREE(hdr);
	SET_PTR_SIZE(hdr, ( blk_size - sizeof(_mem_ptr_hdr) ) );
	
	DPRINTF(5, ("_pool_new_blk() new blk x%lx size %ld memory x%lx pool #%d [x%lx]\n",
			new_blk, new_blk->size, new_blk->memory, pool->id, pool));
	return new_blk;
	}
	

#ifdef DOCUMENTATION

	_pool_find_free_blk() looks for a block in the pool with enough free memory
	to allocate the "size_req" bytes.
	
#endif

_mem_blk_ptr
_pool_find_free_blk(pool, size_req, hdr_ptr)
_mem_pool_ptr		pool;
u_long				size_req;	/* size required */
_mem_ptr_hdr_ptr	*hdr_ptr;
{
_mem_blk_ptr		blk, use_this_blk;
_mem_ptr_hdr_ptr	loophdr, limit;
long				hdrsize, max_size;

#ifdef _PM_DYNAMIC_MERGING
_mem_ptr_hdr_ptr	lasthdr, nexthdr, blkhdr;
long				lastsize, nextsize;
#endif _PM_DYNAMIC_MERGING

	blk = pool->blk_list;
	max_size = 0x3FFFFFFF;
	for (use_this_blk=NULL; blk != (_mem_blk_ptr)0 ; blk=blk->next) {
		if (blk->max_free >= size_req) {
			if (use_this_blk == NULL) {
				use_this_blk = blk;
				max_size = blk->size;
				}
			else if (blk->size < max_size) {
				use_this_blk = blk;
				max_size = blk->size;
				}
			}
		}
	
	blk = use_this_blk;
	if (blk != NULL) {
		loophdr = (_mem_ptr_hdr_ptr) blk->memory;
		limit = (_mem_ptr_hdr_ptr) ((char *)blk->memory + blk->size);
#ifdef _PM_DYNAMIC_MERGING
		lasthdr = (_mem_ptr_hdr_ptr)0;
#endif _PM_DYNAMIC_MERGING
		while (loophdr < limit) {
			if (IS_PTR_FREE(loophdr)) {
				hdrsize = GET_PTR_SIZE(loophdr);
				if (hdrsize >= size_req) {
					DPRINTF(5, ("pool_find_free_blk() Found blk x%lx hdr x%lx pool #%d [x%lx]\n",
								blk, loophdr, pool->id, pool));
					
					*hdr_ptr = loophdr;
					return blk;
					}

#ifdef _PM_DYNAMIC_MERGING
				else {
					nexthdr = (_mem_ptr_hdr_ptr)
								( (char *)loophdr + hdrsize + sizeof(_mem_ptr_hdr) );
					if (nexthdr < limit) {
						if (IS_PTR_FREE(nexthdr)) {
							nextsize = GET_PTR_SIZE(nexthdr);
							blk->max_free += sizeof(_mem_ptr_hdr);	/* We're gonna merge... */
							if ((nextsize + hdrsize + sizeof(_mem_ptr_hdr)) >= size_req) {
								DPRINTF(3, ("pool_find_free_blk() F-Merge blk x%lx hdr1 x%lx hdr2 x%lx \n",
											blk, loophdr, nexthdr));
								
								hdrsize = nextsize + hdrsize + sizeof(_mem_ptr_hdr);
								SET_PTR_SIZE(loophdr, hdrsize);
								*hdr_ptr = loophdr;
								return blk;
								}
							else {
								/* OK, so merge anyways... */
								hdrsize = nextsize + hdrsize + sizeof(_mem_ptr_hdr);
								SET_PTR_SIZE(loophdr, hdrsize);
								}
							}
						}
					if (lasthdr != (_mem_ptr_hdr_ptr)0) {
						if (IS_PTR_FREE(lasthdr)) {
							lastsize = GET_PTR_SIZE(lasthdr);
							blk->max_free += sizeof(_mem_ptr_hdr);	/* We're gonna merge... */
							if ((lastsize + hdrsize + sizeof(_mem_ptr_hdr)) >= size_req) {
								DPRINTF(3, ("pool_find_free_blk() R-Merge blk x%lx hdr1 x%lx hdr2 x%lx \n",
											blk, lasthdr, loophdr));
								
								hdrsize = lastsize + hdrsize + sizeof(_mem_ptr_hdr);
								SET_PTR_SIZE(lasthdr, hdrsize);
								*hdr_ptr = lasthdr;
								return blk;
								}
							else {
								/* OK, so merge anyways... */
								hdrsize = lastsize + hdrsize + sizeof(_mem_ptr_hdr);
								SET_PTR_SIZE(lasthdr, hdrsize);
								}
							loophdr = lasthdr;
							lasthdr = (_mem_ptr_hdr_ptr)0;
							}
						}
#ifdef _PM_DYNAMIC_FREE
					blkhdr = (_mem_ptr_hdr_ptr)blk->memory;
					if (IS_PTR_FREE(blkhdr)) {
						if ((GET_PTR_SIZE(blkhdr) + sizeof(_mem_ptr_hdr)) == blk->size) {
							/* This blk is free-ed ... */
							
							_mem_blk_ptr	tmpblk, next;
			
							if (blk == blk->pool->blk_list) {
								blk->pool->blk_list = next = blk->next;
								}
							else {
								tmpblk = blk->pool->blk_list;
								while (tmpblk != (_mem_blk_ptr)0) {
									if (tmpblk->next == blk) {
										tmpblk->next = next = blk->next;
										break;
										}
									else
										tmpblk = tmpblk->next;
									}
								if (tmpblk == (_mem_blk_ptr)0)
									DPRINTF(1, ("pool_find_free_blk() ERROR: could not free blk x%lx from list!\n", blk));
								}
							
							DPRINTF(3, ("pool_find_free_blk() Freeing Block x%lx from pool #%d!\n",
										blk, blk->pool->id));
#ifdef _PM_STATS
							pool->total_memory -= sizeof(_mem_blk) + blk->size;
							pool->total_storage -= blk->size;
#endif
							mDISPOSPTR((Ptr)blk->memory);
							mDISPOSPTR((Ptr)blk);
							blk = next;
							break;	/* The while (hdr) loop, into while (blk) loop */
							}
						}	/* if (IS_PTR_FREE(blkhdr)) */
#endif _PM_DYNAMIC_FREE

					}
#endif _PM_DYNAMIC_MERGING

				}
#ifdef _PM_DYNAMIC_MERGING
			lasthdr = loophdr;
#endif _PM_DYNAMIC_MERGING
			loophdr = (_mem_ptr_hdr_ptr)
						((char *)loophdr + GET_PTR_SIZE(loophdr) + sizeof(_mem_ptr_hdr));
			}
		}
	
	*hdr_ptr = (_mem_ptr_hdr_ptr)0;
	return blk;
	}
	

#ifdef DOCUMENTATION

	_pool_find_ptr_blk() finds the block containing this pointer in "ptr".
	
#endif

_mem_blk_ptr
_pool_find_ptr_blk(ptr)
char	*ptr;
{
_mem_pool_ptr	pool;
_mem_blk_ptr	blk;

	/*
	** Since the default list is stored at the front of the forest list,
	** we inherently search the default forest first. Nice.
	*/
	pool = _mem_pool_forest;
	
	while (pool != (_mem_pool_ptr)0) {
		blk = pool->blk_list;
		
		while (blk != (_mem_blk_ptr)0) {
			if ( ( ptr >= blk->memory ) &&
				 ( ptr <= ((char *)blk->memory + blk->size) ) )
				return blk;
			else
				blk = blk->next;
			}
		
		pool = pool->next;
		}
	
	return (_mem_blk_ptr)0;
	}


#ifdef DOCUMENTATION

	merge_free_list() runs the routine to merge and "release" any free-ed
	blocks back into the heap.
	
#endif

merge_free_list()
{
	return _pool_free_list_merge(_default_mem_pool);
	}


#ifdef DOCUMENTATION

	_pool_free_list_merge() will merge and "release" any free-ed blocks
	back into the heap.
	
#endif

_pool_free_list_merge(pool)
_mem_pool_ptr		pool;
{
_mem_blk_ptr		blk, nextblk;
_mem_ptr_hdr_ptr	loophdr, limit, nexthdr, blkhdr;
int					result = 0;
long				hdrsize, nextsize;

	blk = pool->blk_list;
	while (blk != (_mem_blk_ptr)0) {
		loophdr = blkhdr = (_mem_ptr_hdr_ptr)blk->memory;
		limit = (_mem_ptr_hdr_ptr) ((char *)blk->memory + blk->size);
		while (loophdr < limit) {
			if (IS_PTR_FREE(loophdr)) {
				hdrsize = GET_PTR_SIZE(loophdr);
				nexthdr = (_mem_ptr_hdr_ptr)
							( (char *)loophdr + hdrsize + sizeof(_mem_ptr_hdr) );
				/* Now loop and merge free ptr's til used one is hit... */
				while (nexthdr < limit) {
					if (IS_PTR_FREE(nexthdr)) {
						nextsize = GET_PTR_SIZE(nexthdr);
						blk->max_free += sizeof(_mem_ptr_hdr);	/* We're gonna merge... */
						hdrsize = nextsize + hdrsize + sizeof(_mem_ptr_hdr);
						SET_PTR_SIZE(loophdr, hdrsize);
						nexthdr = (_mem_ptr_hdr_ptr)
									( (char *)loophdr + hdrsize + sizeof(_mem_ptr_hdr) );
						}
					else
						break;
					}
				}
			
			loophdr = (_mem_ptr_hdr_ptr)
						((char *)loophdr + GET_PTR_SIZE(loophdr) + sizeof(_mem_ptr_hdr));
			}

		/* Get next block now, so _PM_DYNAMIC_FREE can modify blk as desired */
		nextblk = blk->next;

#ifdef _PM_DYNAMIC_FREE
		if (IS_PTR_FREE(blkhdr)) {
			if ((GET_PTR_SIZE(blkhdr) + sizeof(_mem_ptr_hdr)) == blk->size) {
				/* This blk is free-ed ... */
				_mem_blk_ptr	tmpblk;

				if (blk == blk->pool->blk_list) {
					blk->pool->blk_list = blk->next;
					}
				else {
					tmpblk = blk->pool->blk_list;
					while (tmpblk != (_mem_blk_ptr)0) {
						if (tmpblk->next == blk) {
							tmpblk->next = blk->next;
							break;
							}
						else
							tmpblk = tmpblk->next;
						}
					if (tmpblk == (_mem_blk_ptr)0)
						DPRINTF(1, ("pool_find_free_blk() ERROR: could not free blk x%lx from list!\n", blk));
					}
				
				DPRINTF(3, ("pool_find_free_blk() Freeing Block x%lx from pool #%d!\n",
							blk, blk->pool->id));
#ifdef _PM_STATS
				pool->total_memory -= sizeof(_mem_blk) + blk->size;
				pool->total_storage -= blk->size;
#endif
				result += blk->size + sizeof(_mem_blk);
				mDISPOSPTR((Ptr)blk->memory);
				mDISPOSPTR((Ptr)blk);
				}
			
			}	/* if (IS_PTR_FREE(blkhdr)) */
#endif _PM_DYNAMIC_FREE
		
		blk = nextblk;
		}
	
	return result;
	}
	

#ifdef DOCUMENTATION

	get_malloc_stats() return several statistics about the default heap.
	
#endif

get_malloc_stats(total_memory, total_free, total_malloc)
long		*total_memory;
long		*total_free;
long		*total_malloc;
{
_mem_pool_ptr		pool;
_mem_blk_ptr		blk;
_mem_ptr_hdr_ptr	hdr, limit;
u_long				total_size, used_space, free_space;
u_long				ptr_size;

	pool = _default_mem_pool;
	blk = pool->blk_list;
	total_size = used_space = free_space = 0;
	for ( ; blk != (_mem_blk_ptr)0; ) {
		hdr = (_mem_ptr_hdr_ptr) blk->memory;
		limit = (_mem_ptr_hdr_ptr) ((char *)blk->memory + blk->size);
		total_size += blk->size;
		for ( ; hdr && hdr < limit; ) {
			ptr_size = GET_PTR_SIZE(hdr);
			if (IS_PTR_FREE(hdr))
				free_space += ptr_size;
			else
				used_space += ptr_size;
			
			hdr = (_mem_ptr_hdr_ptr)
					( (char *)hdr + GET_PTR_SIZE(hdr) + sizeof(_mem_ptr_hdr) );
			}
		
		blk = blk->next;
		}
	
	*total_memory = total_size;
	*total_free = free_space;
	*total_malloc = used_space;
	}

#ifdef DEBUG

#include <stdio.h>

char	temp_str[512];

list_all_pool_stats(file)
FILE	*file;
{
_mem_pool_ptr		pool;

	pool = _mem_pool_forest;
	
	while (pool != (_mem_pool_ptr)0) {
		
		list_pool_stats(file, pool);

		pool = pool->next;
		}
	
	}


list_pool_stats(file, pool)
FILE	*file;
_mem_pool_ptr		pool;
{
_mem_blk_ptr		blk;
_mem_ptr_hdr_ptr	hdr, limit;
int					i, j;
int					num_used_hdrs, num_free_hdrs;
u_long				used_hdr_space, free_hdr_space;
u_long				ptr_size, max_free_size, max_used_size;
float				ave_used_size, ave_free_size, frag_ratio;

	if (file == (FILE *)0) {
		file = stdout;
		}
	
	fprintf(file, "POOL STATISTICS FOR ID#%d @ x%lx blk_list x%lx pref_blk_size %ld\n",
			pool->id, pool, pool->blk_list, pool->pref_blk_size);

#ifdef _PM_STATS
	fprintf(file, "\tMEMORY: Total memory %ld Total ptr space %ld Currently malloc-ed %ld\n",
			pool->total_memory, pool->total_storage, pool->total_malloc);
	fprintf(file, "\tREQUESTS: Ave req size %f Total reqs %ld Total size reqs %f\n",
			pool->ave_req_size, pool->ave_req_total, 
			(float) (pool->ave_req_size * (float)pool->ave_req_total));
	fprintf(file, "\tBLOCKS: Max block size %ld Ave block size %f\n",
			pool->max_blk_size, pool->ave_blk_size);
	fprintf(file, "\tBLOCKS: Total ave blocks %ld Total ave size %f\n",
			pool->ave_blk_total,(float) (pool->ave_blk_size * (float)pool->ave_blk_total));
#endif

		fprintf(file, "POOL BLOCK STATISTICS:\n");
		
		blk = pool->blk_list;
		for (i=0; blk != (_mem_blk_ptr)0; i++) {
			hdr = (_mem_ptr_hdr_ptr) blk->memory;
			limit = (_mem_ptr_hdr_ptr) ((char *)blk->memory + blk->size);
			num_used_hdrs = num_free_hdrs = 0;
			used_hdr_space = free_hdr_space = 0;
			max_free_size = max_used_size = 0;
			for (j=0; hdr && hdr < limit; j++) {
				ptr_size = GET_PTR_SIZE(hdr);
				if (IS_PTR_FREE(hdr)) {
					num_free_hdrs++;
					free_hdr_space += ptr_size;
					if (ptr_size > max_free_size)
						max_free_size = ptr_size;
					}
				else {
					num_used_hdrs++;
					used_hdr_space += ptr_size;
					if (ptr_size > max_used_size)
						max_used_size = ptr_size;
					}
				
				hdr = (_mem_ptr_hdr_ptr)
						( (char *)hdr + GET_PTR_SIZE(hdr) + sizeof(_mem_ptr_hdr) );
				}
			
			if (num_free_hdrs == 0)
				ave_free_size = (float)0.0;
			else
				ave_free_size = (float)free_hdr_space / (float)num_free_hdrs;
			
			if (num_used_hdrs == 0)
				ave_used_size = (float)0.0;
			else
				ave_used_size = (float)used_hdr_space / (float)num_used_hdrs;
			
			if (num_used_hdrs == 0) {
				frag_ratio = (float)num_free_hdrs / 2.0;
				}
			else {
				frag_ratio = (float)num_free_hdrs / (float)num_used_hdrs;
				}
			
			fprintf(file, "\tBLOCK #%-4d Fragmentation Ratio %5.2f\n", i, frag_ratio);
			fprintf(file, "\t      #%-4d    Block Size %ld Ptr x%lx MaxFree %ld\n",
					i, blk->size, blk->memory, blk->max_free);
			fprintf(file, "\t      #%-4d    Number free ptrs %d Number used ptrs %d\n",
					i, num_free_hdrs, num_used_hdrs);
			fprintf(file, "\t      #%-4d    Free space %ld Used space %ld\n",
					i, free_hdr_space, used_hdr_space);
			fprintf(file, "\t      #%-4d    Max free ptr %ld Max used ptr %ld\n",
					i, max_free_size, max_used_size);
			fprintf(file, "\t      #%-4d    Ave free ptr size %5.1f Ave used ptr size %5.1f\n",
					i, ave_free_size, ave_used_size);

			blk = blk->next;
			}
	}


list_pool_forest(file, with_data)
FILE	*file;
int		with_data;
{
_mem_pool_ptr		pool;
_mem_blk_ptr		blk;
_mem_ptr_hdr_ptr	hdr, limit;
int					i, j;
extern char			temp_str[];

	if (file == (FILE *)0) {
		file = stdout;
		}
	
	pool = _mem_pool_forest;
	while (pool != (_mem_pool_ptr)0) {
		
		list_pool_stats(file, pool);
		
		blk = pool->blk_list;
		for (i=0; blk != (_mem_blk_ptr)0; i++) {
			fprintf(file, "   BLK #%05d ; @ x%lx ; blk_size %ld ; memory x%lx ; max_free %ld\n",
					i, blk, blk->size, blk->memory, blk->max_free);
			fprintf(file, "   BLK          pool x%lx ; next blk x%lx\n", blk->pool, blk->next);
			
			hdr = (_mem_ptr_hdr_ptr) blk->memory;
			limit = (_mem_ptr_hdr_ptr) ((char *)blk->memory + blk->size);
			for (j=0; hdr && hdr < limit; j++) {
				sprintf(temp_str, "      PTR #%d ; hdr x%lx ; nxt x%08lx ; size %ld ; flgs x%02x",
						j, hdr, hdr->size, GET_PTR_SIZE(hdr), GET_PTR_FLAGS(hdr));
				if (with_data)
					hex_dump(file, temp_str,
							(char *)((char *)hdr + sizeof(_mem_ptr_hdr)),
							GET_PTR_SIZE(hdr));
				else
					fprintf(file, "%s\n", temp_str);
				
				hdr = (_mem_ptr_hdr_ptr)
						( (char *)hdr + GET_PTR_SIZE(hdr) + sizeof(_mem_ptr_hdr) );
				}
			
			blk = blk->next;
			}
		
		pool = pool->next;
		}
	}

#define ROW_BYTES		16

hex_dump(output, title, ptr, bytes)
FILE	*output;
char	*title;
char	*ptr;
long	bytes;
{
int				rows, residue, i, j;
unsigned char	save_buf[ROW_BYTES+2];
unsigned char	hex_buf[4];
char			hex_chars[20];
	strcpy(hex_chars, "0123456789ABCDEF");
	
	fprintf(output, "\n%s - x%lX (%ld) bytes.\n", title, bytes, bytes);
	rows = bytes >> 4;
	residue = bytes & 0x0000000F;
	for (i=0; i<rows; i++) {
		fprintf(output, "%04.4X:", i * ROW_BYTES);
		for (j=0; j<ROW_BYTES; j++) {
			save_buf[j] = *ptr++;
			hex_buf[0] = hex_chars[(save_buf[j] >> 4) & 0x0F];
			hex_buf[1] = hex_chars[save_buf[j] & 0x0F];
			hex_buf[2] = '\0';
			fprintf(output, " %2.2s", hex_buf);
			if (save_buf[j] < 0x20 || save_buf[j] > 0xD9) save_buf[j] = '.';
			}
		save_buf[ROW_BYTES+1] = '\0';
		fprintf(output, "\t/* %16.16s */\n", save_buf);
		}
	if (residue) {
		fprintf(output, "%04.4X:", i * ROW_BYTES);
		for (j=0; j<residue; j++) {
			save_buf[j] = *ptr++;
			hex_buf[0] = hex_chars[(save_buf[j] >> 4) & 0x0F];
			hex_buf[1] = hex_chars[save_buf[j] & 0x0F];
			hex_buf[2] = '\0';
			fprintf(output, " %2.2s", hex_buf);
			if (save_buf[j] < 0x20 || save_buf[j] > 0xD9) save_buf[j] = '.';
			}
		for (/*j INHERITED*/; j<ROW_BYTES; j++) {
			save_buf[j] = ' ';
			fprintf(output, "   ");
			}
		save_buf[ROW_BYTES+1] = '\0';
		fprintf(output, "\t/* %16.16s */\n", save_buf);
		}
	}

#ifdef TESTING

main(argc, argv)
char	*argc;
char	*argv[];
{
int		i;
_mem_pool_ptr pool;
char	*ptr, *aptr[20];
char	input[128];
#pragma unused (argc, argv)

	printf("Debug level?\n");
	gets(input);
	if (input[0] == '\0' || input[0] == 'q')
		return 0;
	
	pool_malloc_debug_level = atoi(input);
	printf("Debug level set to %d\n", pool_malloc_debug_level);
	
	fprintf(stderr, "******************** START ********************\n");
	list_pool_forest(stderr, 0);
	
	printf("Allocating in default (system) pool...\n");
	for (i = 10 ; i < (10 * 1024) ; i += 128)
		if (malloc(i) == NULL)
			break;
	
	fprintf(stderr, "******************** ## 1 ## ********************\n");
	list_pool_forest(stderr, 0);
	
	printf("Allocating and Free-ing in default (system) pool...\n");
	for (i = 10 ; i < (10 * 1024) ; i += 128)
		if ((ptr = malloc(i)) != NULL)
			free(ptr);
		else
			break;
		
	fprintf(stderr, "******************** ## 2 ## ********************\n");
	list_pool_forest(stderr, 0);
	
	printf("Allocating and Free-ing again in default (system) pool...\n");
	for (i = 0 ; i < 20 ; i++)
		aptr[i] = NULL;
	for (i = 0 ; i < 20 ; i++) {
		aptr[i] = malloc(i * 128);
		if (aptr[i] == NULL)
			break;
		}
	for (i = 19 ; i >= 0 ; i--)
		if (aptr[i] != NULL) {
			free(aptr[i]);
			}
		
	fprintf(stderr, "******************** ## 3 ## ********************\n");
	list_pool_forest(stderr, 0);
	
	pool = new_malloc_pool ( 2001, (16 * 1024) );
	printf("new_malloc_pool ( 2001, %d ) returns x%lx\n", (16 * 1024), pool);
	if (pool) {
		set_default_pool ( 2001 );
		
		printf("Allocating in pool #2001...\n");
		for (i = 10 ; i < (10 * 1024) ; i += 128)
			if (malloc(i) == NULL)
				break;
		}

	fprintf(stderr, "******************** ## 4 ## ********************\n");
	list_pool_forest(stderr, 0);
	
	printf("Free-ing memory in pool #2001...\n");
	free_pool_memory ( 2001 );

	fprintf(stderr, "******************** ## 5 ## ********************\n");
	list_pool_forest(stderr, 0);
	
	printf("Done.\n");
	}

#endif TESTING

#endif _PM_STATS

bzero(ptr, size)
register char *ptr;
register u_long size;
{
	while (size-- > 0) *ptr++ = '\0';
	}

