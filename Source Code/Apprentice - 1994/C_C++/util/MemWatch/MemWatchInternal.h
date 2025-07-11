/*
 * AAMemWatch.h - watch memory allocation freeing
 * by Aaron Wohl (n3liw+@cmu.edu)
 * public domain no rights reserved
 */

#pragma once

#ifndef MemWatchInternal
#define MemWatchInternal

#include "bbt.h"
#include "CObject.h"

/*
 * record the birth of a new object
 * called with c linkage
 */
void mem_record_birth_c(CObject *new_object);

/*
 * remove memory allocation watching hooks
 */
void mem_RemovePatch(void);

/*
 * install memory watching hooks
 */
void mem_InstallPatch(void);

/*
 * record of recently entered proceedures
 */
typedef unsigned char *mem_recent_procs;
typedef mem_recent_procs *mem_recent_procs_pt;

/*
 * for each allocated block remember this many reciently entered proceedures
 */
#define MEM_RECENT_SIZE (7)

/*
 * for each allocated block record the calling proceedures to this depth
 */
#define MEM_STACK_SIZE	(7)

/*
 * structure of a block to record info about each allocated handle or pointer
 */
struct mem_entry_R {
	long mem_addr;		/*memory address that this block is for*/
	long mem_create_time; /*tickcount of time when we where created*/
#define MFL_isptr (1)	/*if set block is a pointer else relocatable*/
#define MFL_isobject (2) /*set if this is a TCL object*/
	long mem_flags;
	mem_recent_procs rp[MEM_RECENT_SIZE]; /*some recently called proceedures*/
	mem_recent_procs stackp[MEM_STACK_SIZE]; /*call stack*/
	char *desc;			/*description from describe or heap scan*/
	unsigned long desc_type;	/*type of object for display*/
	CObject *owner;		/*TCL object that owns this memory*/
	char *class_name;	/*non zero if this is an object, has objects class name*/
};
typedef struct mem_entry_R mem_entry,*mem_entry_pt;

/*
 * return a record of the recently entered proceedures
 */
void mem_get_recent_procs(mem_recent_procs_pt rprocs,unsigned long max_procs);

#define MEM_FIRST_BLOCK ((unsigned long)-1)

/*
 * display a dump of memory blocks and who allocated them
 */
void mem_dump_memory_to_file(char *fname,unsigned long first_block);

/*
 * return the by creation time table of current memory blocks
 */
bbt_table_pt mem_get_by_time(void);

/*
 * return the by address table of current memory blocks
 */
bbt_table_pt mem_get_by_addr(void);

/*
 * return the current memory block creation serial number
 * it is a count of the number of blocks created since startup
 */
unsigned long mem_get_epoch(void);

/*
 * write one memory block to a text buffer
 */
void mem_dump_block(char *dst,mem_entry_pt ablock,long *tot_size);

/*
 * hook called from applicaiton SetupMenus
 */
void mem_menu_setup_hook(void);

/*
 * handle debugging commands
 */
int mem_DoCommand(long the_cmd);

/*
 * is memory debugging on?
 */
int mem_is_debug_on(void);

/*
 * update memu hook
 */
void mem_update_menu_hook(void);

/*
 * is this address in the application heap
 */
int mem_in_my_heap(unsigned long addr);

/*
 * check to see if the passed pstring is the last entered proceedure
 */
int mem_last_proc_is(unsigned char *s);

/*
 * return the number of unalocated memory recording blocks
 */
long mem_blocks_left(void);

/*
 * some low memory variables give hints as to what heap objects are
 */
void mem_make_up_descriptions(void);

/*
 * set a description of block if it is not already set
 */
void mem_try_describe(void *blk,char *describe,unsigned long desc_type);

/*
 * install memory patches if debugging
 */
void mem_MaybeInstallPatch(void);
#endif

