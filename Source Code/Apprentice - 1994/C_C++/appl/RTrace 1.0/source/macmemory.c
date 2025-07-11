/*****************************************************************************\
* macmemory.c                                                                 *
*                                                                             *
* This file contains code which is specific to the Macintosh.  It contains    *
* procedures which handle memory allocation and deallocation.                 *
\*****************************************************************************/

#include <stdlib.h>
#include <GestaltEqu.h>
#include "macerrors.h"


/* Blocks are arranged in a two-way linked list.  Each block consists of an
	eight-byte header, which points to the previous and next blocks, and
	of the data itself */
typedef
struct block_struct
	{
	struct block_struct	*previous;
	struct block_struct	*next;
	long	data;
	} block;

#define	BLOCK_HEADER_SIZE	2*sizeof(block *)

/* externals */
extern Boolean scene_in_memory;		/* TRUE if the scene for the current file is in memory */

/* globals */
block	*last_block;					/* pointer to the last block in the list */
block	*first_block;					/* pointer to the first block in the list */

/* Prototypes */
void *mac_alloc(unsigned int size);
void mac_free(void *p);
void free_all(void);
void init_mac_alloc(void);
void update_status_free_memory (void);


/*****************************************************************************\
* procedure init_mac_alloc                                                    *
*                                                                             *
* Purpose: This procedure initializes the mac-specific memory allocation list *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 1, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Greg Ferrar  9/5/92           Changed mem. management to use two-way list *
\*****************************************************************************/

void init_mac_alloc(void)
{

	short	error;

	/* allocate eight bytes for the first block, and eight more for the
		last block */
	first_block = (block *) NewPtr(BLOCK_HEADER_SIZE);
	if (error = MemError())	abortive_error(error);
	last_block = (block *) NewPtr(BLOCK_HEADER_SIZE);
	if (error = MemError())	abortive_error(error);

	/* Link the last block to the first block, and vice versa */
	last_block->previous = first_block;
	first_block->next = last_block;
	
	/* Mark these blocks as the ends of the list */
	last_block->next = first_block->previous = NULL;

}	/* init_mac_alloc() */



/*****************************************************************************\
* procedure mac_alloc                                                         *
*                                                                             *
* Purpose: This procedure allocates a block of memory.  This block can be     *
*          released either by calling mac_free or by calling free_all.        *
*                                                                             *
* Parameters: size:   size of block                                           *
*             returns pointer to allocation                                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 25, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Greg Ferrar  9/1/92           Changed memory management to use list       *
*   Greg Ferrar  9/5/92           Changed mem. management to use two-way list *
\*****************************************************************************/

void *mac_alloc(unsigned int size)
{

	register block		*p;

	/* Allocate the memory */
	p = (block *) malloc (size + BLOCK_HEADER_SIZE);

	if (p)	/* allocated */
		{
		
		/* Insert this block between last_block and the one before it */
		p->next = last_block;
		p->previous = last_block->previous;
		last_block->previous = p;
		p->previous->next = p;
		
		}
	
	else	/* didn't allocate */
		{

		/* We have to remove the scene from memory, or there won't be enough
			memory to display the error dialog */
		free_all();
		
		/* Update the free memory string */
		update_status_free_memory();
		
		/* Show the error dialog and abort */
		abortive_string_error ("Error: not enough memory to finish processing scene.");

		}
	
	/* return a pointer to the data */
	return &(p->data);

}	/* mac_alloc() */



/*****************************************************************************\
* procedure mac_free                                                          *
*                                                                             *
* Purpose: This procedure frees a block of memory which was allocated using   *
*          mac_alloc.                                                         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 25, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Greg Ferrar  9/1/92           Changed memory management to use list       *
*   Greg Ferrar  9/5/92           Changed mem. management to use two-way list *
\*****************************************************************************/

void mac_free(void *p)
{

	register block	*the_block; 

	/* Find the start of the block */
	the_block = (block *) ( ((char *) p) - BLOCK_HEADER_SIZE );

	/* Remove the block from the list */
	the_block->previous->next = the_block->next;
	the_block->next->previous = the_block->previous;
	
	/* Free the block */
	free(the_block);

}	/* mac_free() */



/*****************************************************************************\
* procedure free_all                                                          *
*                                                                             *
* Purpose: This procedure frees all memory of which was previously allocated  *
*          by mac_alloc.                                                      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 25, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Greg Ferrar  9/1/92           Changed memory management to use list       *
*   Greg Ferrar  9/5/92           Changed mem. management to use two-way list *
\*****************************************************************************/

void free_all(void)
{

	block 	*current_block;
	block	*previous_block;

	/* Start with the block before the last */
	current_block = last_block->previous;

	/* Keep freeing the last item in the list until there's nothing left except
		the first block and the last block */
	while (current_block != first_block)
		{
		
		/* Find the link from the current block to the previous one */
		previous_block = current_block->previous;
		
		/* Free the current block, since it's not the last one */
		free(current_block);
		
		/* Go on the the previous block */
		current_block = previous_block;
		
		}
	
	/* Connect the last block to the first block */
	last_block->previous = first_block;
	first_block->next = last_block;

	/* The scene is no longer in memory */
	scene_in_memory = TRUE;

}	/* free_all() */